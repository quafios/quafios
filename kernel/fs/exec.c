/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> Filesystem: execve() system call.                | |
 *        | +------------------------------------------------------+ |
 *        +----------------------------------------------------------+
 *
 * This file is part of Quafios 1.0.2 source code.
 * Copyright (C) 2014  Mostafa Abd El-Aziz Mohamed.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Quafios.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Visit http://www.quafios.com/ for contact information.
 *
 */

#include <arch/type.h>
#include <lib/string.h>
#include <sys/mm.h>
#include <sys/error.h>
#include <sys/fs.h>
#include <sys/scheduler.h>
#include <arch/stack.h>
#include <std/elf.h>

/* string vector might be "argv" or "envp" passed to the new program. */

int32_t strv_len(char *strv[]) {

    int32_t count = 0;

    /* NULL? */
    if (strv == (char **) 0)
        return 0;

    /* count: */
    while(strv[count] != (char *) 0)
        count++;

    /* return: */
    return count;

}

char **store_strv(char *strv[]) {

    /* function to store vector of strings in kernel memory. */
    int32_t count = strv_len(strv), i;

    /* allocate kernel memory: */
    char **kstrv = (char **) kmalloc((count+1)*sizeof(char *));

    /* copy every string to kernel memory: */
    for (i = 0; i < count; i++) {
        int32_t len = strlen(strv[i]);
        char *dest = (char *) kmalloc(len+1);
        strcpy(dest, strv[i]);
        kstrv[i] = dest;
    }
    kstrv[i] = (char *) 0;

    /* return */
    return kstrv;

}

uint32_t restore_strv(char *strv[], uint32_t stack_top) {

    /* restore string vector to stack.
     * this function returns a new stack top
     * which equals stack_top - size_of_copied_data
     * it is in the same time pointer to argv
     * in the stack.
     */

    int32_t count = strv_len(strv), i;

    /* copy every string from kernel memory to stack: */
    for (i = count-1; i >= 0; i--) {
        /* length and offset: */
        int32_t len = strlen(strv[i]);
        stack_top -= (len+1);

        /* copy string: */
        strcpy((char *) stack_top, strv[i]);

        /* deallocate kernel memory: */
        kfree(strv[i]);

        /* update strv: */
        strv[i] = (char *) stack_top;
    }

    /* copy the vector itself: */
    for (i = count; i >= 0; i--) {
        stack_top -= sizeof(char *);
        *((char **) stack_top) = strv[i];
    }

    /* deallocate kernel memory: */
    kfree(strv);

    /* return the new stack top: */
    return stack_top;
}


/****************************************************************************/
/*                                 execve()                                 */
/****************************************************************************/

int32_t execve(char *filename, char *argv[], char *envp[]) {

    int32_t err, i;
    uint32_t sp;
    file_t *file;
    size_t done;
    Elf32_Ehdr header = {0};
    Elf32_Phdr pheader;
    Elf32_Off  offset;
    Elf32_Addr vaddr;
    Elf32_Word filesz;
    Elf32_Word memsz;
    char *shebang = (char *) &header;

    /* Open the executable file:  */
    /* -------------------------- */
    if (err = file_open(filename, 0, &file))
        return -err;

    /* not a regular file? */
    if ((file->inode->mode & FT_MASK) != FT_REGULAR) {
        file_close(file);
        return -EINVAL;
    }

    /* Read the ELF header:  */
    /* --------------------- */
    err = file_read(file, &header,sizeof(header),&done);
    if (err) {
        file_close(file);
        printk("execve(): Error on reading header\n");
        return err ? (-err) : (-EINVAL);
    };

    /* Identify shebang file:  */
    /* ----------------------- */
    if (shebang[0] == '#' && shebang[1] == '!') {
        char *argv[3];
        int32_t i;
        argv[0] = &shebang[2];
        argv[1] = filename;
        argv[2] = 0;
        for (i = 0; i < sizeof(header)-1; i++) {
            if (shebang[i] == '\n')
                break;
        }
        shebang[i] = 0;
        return execve(&shebang[2], argv, NULL);
    }

    /* Indetify ELF file:  */
    /* ------------------- */
    if (header.e_ident[EI_MAG0 ] != 0x7F || /* Check Magic ELF signature. */
        header.e_ident[EI_MAG1 ] != 'E'  ||
        header.e_ident[EI_MAG2 ] != 'L'  ||
        header.e_ident[EI_MAG3 ] != 'F'  ||
        header.e_ident[EI_CLASS] != ELFCLASS32  ||
        header.e_ident[EI_DATA ] != ELFDATA2LSB || /* Only support LSB. */
        header.e_type != ET_EXEC || /* Only load executable files. */
        header.e_machine != EM_386) {
            /* We only support Intel 386 32-bit programs. */
            file_close(file);
            printk("execve(): Invalid ELF file\n");
            return -EINVAL;
    }

    /* store argv & envp in kernel memory: */
    argv = store_strv(argv);
    envp = store_strv(envp);

    /* unmap all memory regions allocated by current processes. */
    umem_free(&(curproc->umem));

    /* create a new memory image */
    umem_init(&(curproc->umem));

    /* switch to the new memory image */
    arch_vmswitch(&(curproc->umem));

    /* map a user stack: */
    mmap(USER_MEMORY_END-USER_STACK_SIZE, USER_STACK_SIZE,
         MMAP_TYPE_ANONYMOUS, 0, 0, 0);

    /* Copy data to the just-created stack:  */
    /* ===================================== */

    /* stack style:
     * --------------
     *        *****************
     *        *  env strings  *
     *        *****************
     *        *  env vector   *<<---+
     *        *****************     |
     *        *  arg strings  *     |
     *        *****************     |
     *  +--->>*  arg vector   *     |
     *  |     *****************     |
     *  |     *     envp      *-----+
     *  |     *****************
     *  +-----*     argv      *
     *        *****************
     *        *     argc      *
     *        *****************
     *        *      pc       *
     *        *****************
     */

    /* push environment strings & vector to stack: */
    envp = (char **) restore_strv(envp, USER_MEMORY_END);

    /* push arg strings & arg vector to stack (down env vector area): */
    argv = (char **) restore_strv(argv, (uint32_t) envp);

    /* push envp (down arg vector area) */
    sp = (uint32_t) argv;
    sp -= sizeof(char **);
    *((char ***) sp) = envp;

    /* push argv: */
    sp -= sizeof(char **);
    *((char ***) sp) = argv;

    /* push argc: */
    sp -= sizeof(int32_t);
    *((int32_t *) sp) = strv_len(argv);

    /* push a pseudo program counter: */
    sp -= sizeof(void *);
    *((void **) sp) = (void *) 0;

    /* Copy the program itself:  */
    /* ========================= */
    for (i = 0; i < header.e_phnum; i++) {
        pos_t newpos = (pos_t)(header.e_phoff+(header.e_phentsize*i));
        file_seek(file, newpos, NULL, SEEK_SET);
        file_read(file, &pheader, sizeof(pheader), &done);

        if (pheader.p_type != PT_LOAD)
            continue;

        offset = pheader.p_offset;
        vaddr  = pheader.p_vaddr;
        filesz = pheader.p_filesz;
        memsz  = pheader.p_memsz;

        /* allocate memory for the region... */
        mmap(vaddr, memsz, MMAP_TYPE_ANONYMOUS, 0, 0, 0);

        /* load data from the file to the region: */
        newpos = offset;
        file_seek(file, newpos, NULL, SEEK_SET);
        file_read(file, (char *) vaddr, filesz, &done);

        /* update heap start: */
        if (vaddr + memsz > curproc->umem.heap_start)
                curproc->umem.heap_start = vaddr + memsz;
    }

    /* update heap parameters: */
    curproc->umem.brk_addr = curproc->umem.heap_start;
    curproc->umem.heap_end = USER_MEMORY_END-USER_STACK_SIZE;

    /* program loaded. */
    file_close(file);

    /* jump to the entry point... */
    umode_jmp(header.e_entry, sp);

}
