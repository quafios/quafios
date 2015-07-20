/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> memman: user memory.                             | |
 *        | +------------------------------------------------------+ |
 *        +----------------------------------------------------------+
 *
 * This file is part of Quafios 2.0.1 source code.
 * Copyright (C) 2015  Mostafa Abd El-Aziz Mohamed.
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
#include <lib/linkedlist.h>
#include <sys/error.h>
#include <sys/mm.h>
#include <sys/fs.h>
#include <sys/scheduler.h>
#include <arch/page.h>

int32_t umem_init(umem_t *umem) {

    /* This function is called on creating a new process :D
     * every process has a virtual memory... the struct
     * umem_t holds data that describes the vmem.
     */

    /* initialize heap: */
    umem->heap_start = 0;
    umem->brk_addr   = 0;
    umem->heap_end   = 0;

    /* initialize arch-dependant stuff; */
    return arch_vminit(umem);

}

int32_t umem_copy(umem_t *src, umem_t *dest) {

    /* used by fork() to copy src into dest. */
    uint32_t i, j;
    uint8_t *buf1, *buf2;

    buf1 = (uint8_t *) kmalloc(PAGE_SIZE);
    if (buf1 == NULL)
        return ENOMEM;

    buf2 = (uint8_t *) kmalloc(PAGE_SIZE);
    if (buf2 == NULL) {
        kfree(buf1);
        return ENOMEM;
    }

    /* copy umem; */
    for (i=USER_MEMORY_BASE; i<KERNEL_MEMORY_BASE; i+=PAGE_DIR_SIZE) {

        if (!arch_vmdir_isMapped(src, i))
            continue;

        for (j = i; j < i + PAGE_DIR_SIZE; j+=PAGE_SIZE) {
            if (!arch_vmpage_isMapped(src, j))
                continue;
            arch_vmpage_map(dest, (int32_t) j, 1 /* user mode */);
            arch_vmpage_copy(src, j, dest, j, buf1, buf2);
        }

    }

    /* free the buffers: */
    kfree(buf1);
    kfree(buf2);

    /* copy heap parameters: */
    dest->heap_start = src->heap_start;
    dest->brk_addr   = src->brk_addr;
    dest->heap_end   = src->heap_end;

    /* done: */
    return ESUCCESS;

}

void umem_free(umem_t *umem) {

    /* re-initialize heap: */
    umem->heap_start = 0;
    umem->brk_addr   = 0;
    umem->heap_end   = 0;

}

uint32_t mmap(uint32_t base, uint32_t size, uint32_t type,
              uint32_t flags, uint32_t fd, uint64_t off) {

    int32_t pages;
    uint32_t addr;
    umem_t *umem = &(curproc->umem); /* current process umem image. */

    /*printk("mmap called: %x, size: %x\n", base, size); */

    /* make sure fd is valid if a file is to be used */
    if (type & MMAP_TYPE_FILE) {
        if (fd < 0 || fd >= FD_MAX || curproc->file[fd] == NULL)
            return 0;
    }

    if (!base) {
        /* allocate space */
        base = umem->heap_end - size;
    }

    size += base & (~PAGE_BASE_MASK);     /* rectify "size". */
    base = base & PAGE_BASE_MASK;
    pages = (size+PAGE_SIZE-1)/PAGE_SIZE; /* pages to be allocated. */
    size = pages*PAGE_SIZE;               /* actual size. */

    /*printk("           - %x, size: %x\n", base, size); */

    /* make sure the system is initialized. */
    if (curproc == NULL)
        return 0; /* system is not initialized. */

    /* check whether the given range is valid or not. */
    for (addr = base; addr < base + size; addr+=PAGE_SIZE) {
        if (addr<USER_MEMORY_BASE || addr>=KERNEL_MEMORY_BASE) {
            return 0; /* invalid */
        }
    }

    /* update heap end: */
    if (base < umem->heap_end)
        umem->heap_end = base;

    /* now allocate. */
    for (addr = base; addr < base + size; addr+=PAGE_SIZE) {
        arch_vmpage_map(umem, (int32_t) addr, 1 /* user mode */);
        /* mapping a file? */
        if (type & MMAP_TYPE_FILE) {
            /* read file information */
            file_t *file = curproc->file[fd];
            inode_t *inode = file->inode;
            file_mem_t *region = inode->sma.first;

            /* shared? */
            if (flags & MMAP_FLAGS_SHARED) {
                /* search for the wanted region */
                while (region != NULL) {
                    if (region->pos == off) {
                        /* found */
                        region->ref++;
                        break;
                    }
                    region = region->next;
                }

                /* region found or not? */
                if (!region) {
                    /* region not found, create it */
                    region = kmalloc(sizeof(file_mem_t));
                    if (!region)
                        return 0;
                    if (file_reopen(file, &(region->file)))
                        return 0;
                    region->pos = off;
                    region->paddr = 0;
                    region->ref = 1;

                    /* add to the inode */
                    linkedlist_add(&(inode->sma), region);
                }
            } else {
                /* not shared, allocate a new one */
                region = kmalloc(sizeof(file_mem_t));
                if (!region)
                    return 0;
                if (file_reopen(file, &(region->file)))
                    return 0;
                region->pos = off;
                region->paddr = 0;
                region->ref = 0;
            }
            /* attach the virtual page to the mapping */
            arch_vmpage_attach_file(umem, (int32_t) addr, region);
            /* update offset */
            off += PAGE_SIZE;
        }
    }

    /* return the base address. */
    return base;
}

int32_t munmap() {


}

uint32_t brk(uint32_t addr) {

    /* change the value of break address. this increases
     * or decreases data segment size.
     */

    uint32_t first_page, last_page, i;

    /* receive umem structure of current process: */
    umem_t *umem = &(curproc->umem);

    /* check limits: */
    if (addr >= umem->heap_end || addr < umem->heap_start)
        /* no memory :( */
        return umem->brk_addr; /* just return current break. */

    /* compare addr with current break: */
    if (addr > umem->brk_addr) {
        /* move forward (map) */
        first_page = (umem->brk_addr+PAGE_SIZE-1) & PAGE_BASE_MASK;
        last_page  = (addr-1) & PAGE_BASE_MASK;
        for (i = first_page; i <= last_page; i+=PAGE_SIZE)
            arch_vmpage_map(umem, i, 1 /* user mode */);
    } else if (addr < umem->brk_addr) {
        /* move backwards (umap) */
        first_page = (addr+PAGE_SIZE-1) & PAGE_BASE_MASK;
        last_page  = (umem->brk_addr-1) & PAGE_BASE_MASK;
        for (i = first_page; i <= last_page; i+=PAGE_SIZE)
            arch_vmpage_unmap(umem, i);
    }

    /* set & return the new break: */
    return umem->brk_addr = addr;

}
