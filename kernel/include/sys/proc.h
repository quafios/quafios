/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Process management header.                       | |
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

#ifndef PROC_H
#define PROC_H

#include <arch/spinlock.h>
#include <lib/linkedlist.h>
#include <sys/mm.h>
#include <sys/fs.h>
#include <sys/ipc.h>

typedef struct pd_s {
    struct pd_s *next;
    struct proc_s *proc;
} pd_t; /* process descriptor. used in queues & linked lists. */

typedef _linkedlist(pd_t) pdlist_t;

#define ENQUEUE(list, item) (__extension__({                    \
            linkedlist_addlast((linkedlist *) &(list),          \
                               (linknode   *) &(item));         \
        }))

#define DEQUEUE(list) (__extension__({                          \
            linknode *__ret = (linknode *) (list).first;        \
            linkedlist_remove((linkedlist *) &(list),           \
                              (linknode   *) __ret,             \
                              NULL);                            \
            __ret;                                              \
        }))

typedef struct proc_s {

    /* Process Descriptors: */
    pd_t plist; /* Process descriptor used by process linked list. */
    pd_t sched; /* Process descriptor used by scheduler queues.    */
    pd_t irqd;  /* Process descriptor used by IRQ queues.          */
    pd_t semad; /* Process descriptor used by semaphores           */

    /* Process ID: */
    int32_t pid;

    /* parent: */
    struct proc_s *parent;

    /* Memory: */
    umem_t umem;

    /* File Descriptors: */
    file_t *file[FD_MAX];

    /* Current Working Directory: */
    file_t *cwd;

    /* kernel stack */
    uint8_t *kstack;

    /* when a syscall is called: */
    void *context;

    /* used by the scheduler: */
    int32_t  after_fork;
    uint32_t reg1;
    uint32_t reg2;

    /* message inbox */
    int32_t inbox_lock;
    int32_t blocked_for_msg;
    _linkedlist(msg_t) inbox;

    /* children */
    int32_t blocked_for_child;

    /* blocked? */
    int32_t blocked;
    spinlock_t *lock_to_unlock;

    /* exit status: */
    int32_t terminated;
    int32_t status;

} proc_t;

extern pdlist_t proclist;
extern int32_t last_pid;
extern proc_t *initproc;

proc_t *get_proc(int32_t pid);

#endif
