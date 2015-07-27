/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Process manager initialization.                  | |
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
#include <sys/printk.h>
#include <sys/scheduler.h>
#include <sys/proc.h>
#include <sys/mm.h>
#include <sys/fs.h>
#include <arch/stack.h>

void proc_init() {

    /* Process Manager Initialization */
    int32_t i, err = 0;
    char *initpath = "/bin/init";
    int32_t bootdisk;

    /* (I) Initialize linked lists:  */
    /* ----------------------------- */
    linkedlist_init((linkedlist *) &proclist);
    linkedlist_init((linkedlist *) &q_ready);
    linkedlist_init((linkedlist *) &q_blocked);

    /* (II) Create "init" process:  */
    /* ---------------------------- */
    /* Allocate memory for process structures: */
    initproc = (proc_t *) kmalloc(sizeof(proc_t));

    /* set parent */
    initproc->parent = NULL;

    /* initialize descriptors: */
    initproc->plist.proc = initproc;
    initproc->sched.proc = initproc;
    initproc->irqd.proc  = initproc;
    initproc->semad.proc = initproc;

    /* Set "init" pid <1>: */
    initproc->pid = 1;

    /* Add it to process list: */
    linkedlist_addlast((linkedlist *) &proclist,
                       (linknode   *) &(initproc->plist));

    /* Kernel-mode stack: */
    initproc->kstack = kernel_stack;

    /* User memory: */
    umem_init(&(initproc->umem));

    /* initialize file descriptors: */
    for(i = 0; i < FD_MAX; i++)
        initproc->file[i] = NULL;

    /* current working directory: */
    file_open("/", 0, &(initproc->cwd));

    /* not forked: */
    initproc->after_fork = 0;

    /* initialize inbox */
    initproc->inbox_lock = 0;
    initproc->blocked_for_msg = 0;
    linkedlist_init(&(initproc->inbox));

    /* children */
    initproc->blocked_for_child = 0;

    /* not blocked */
    initproc->blocked = 0;
    initproc->lock_to_unlock = NULL;

    /* exit status: */
    initproc->terminated = 0;
    initproc->status = 0;

    /* (III) Run the "init" process:  */
    /* ------------------------------ */
    curproc = initproc; /* init process is running! */
    arch_vmswitch(&(initproc->umem));

    /* (IV) Enable multitasking:  */
    /* -------------------------- */
    scheduler_enabled = 1; /* start multitasking! */

    /* (V) Open console streams:  */
    /* -------------------------- */
    /* create device file for console: */
    mknod("/console", FT_SPECIAL, system_console->devid);

    /* open console file: */
    open("/console", 0); /* this will open the file at fd "0". */
    dup(0); /* duplicate at fd "1". */
    dup(0); /* duplicate at fd "2". */

    /* (VI) Mount boot disk:  */
    /* ---------------------- */
    /* detect bootdisk */
    bootdisk = detect_bootdisk();
    if (bootdisk < 0) {
        printk("%aError: Couldn't detect boot medium.\n", 0x0C);
        printk("Kernel halt.%a", 0x0F);
        while(1);
    }

    /* create a device file for bootdisk */
    mknod("/bootdisk", FT_SPECIAL, bootdisk);

    /* mount as diskfs filesystem */
    mount("/bootdisk", "/", "diskfs", 0, NULL);

    /* chdir to the new root */
    chdir("/");

    /* (VII) Execute "init" program to initialize the operating system:  */
    /* ---------------------------------------------------------------- */
    /*synctest();*/
    printk("Executing \"%s\" ...\n\n", initpath);
    printk("%a", 0x0E);
    execve(initpath, 0, 0);
    printk("FATAL: Error happened during execution.\n");
    idle();

}
