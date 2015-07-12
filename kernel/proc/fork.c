/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> procman: fork() system call.                     | |
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
#include <sys/proc.h>
#include <sys/scheduler.h>
#include <sys/fs.h>
#include <arch/stack.h>

/***************************************************************************/
/*                                 fork()                                  */
/***************************************************************************/

int32_t fork() {
    /* Time to... fork!
     * I had spent a long time preparing for this!
     * now I am done, it's time to start working on fork().
     */
    int32_t i;
    proc_t *newproc;

    /* create a new process structure: */
    newproc = kmalloc(sizeof(proc_t));
    if (newproc == NULL)
        return -1;

    /* set parent */
    newproc->parent = curproc;

    /* initialize descriptors: */
    newproc->plist.proc = newproc;
    newproc->sched.proc = newproc;
    newproc->irqd.proc  = newproc;

    /* create memory: */
    if (umem_init(&(newproc->umem))) {
        kfree(newproc);
        return -1; /* error. */
    }

    /* inherit parent's memory: */
    if (umem_copy(&(curproc->umem), &(newproc->umem))) {
        umem_free(&(newproc->umem));
        kfree(newproc);
        return -1; /* error. */
    }

    /* create a new kernel stack. */
    newproc->kstack = (unsigned char *) kmalloc(KERNEL_STACK_SIZE);
    if (newproc->kstack == NULL) {
        umem_free(&(newproc->umem));
        kfree(newproc);
        return -1; /* error. */
    }

    /* initialize kernel stack...
     * this invokes page faults to allocate memory for
     * the stack early.
     */
    for (i = 0; i < KERNEL_STACK_SIZE; i++)
        newproc->kstack[i] = 0;

    /* copy context from parent's stack to child's stack: */
    copy_context(newproc);

    /* copy the set of file descriptors: */
    for (i = 0; i < FD_MAX; i++) {
        if (curproc->file[i] == NULL) {
            newproc->file[i] = NULL;
        } else {
            curproc->file[i]->fcount++;
            newproc->file[i] = curproc->file[i];
        }
    }

    /* inherit the current working directory: */
    curproc->cwd->fcount++;
    newproc->cwd = curproc->cwd;

    /* set pid: */
    newproc->pid = ++last_pid;

    /* inform the scheduler that this is a just-forked process: */
    newproc->after_fork = 1;

    /* initialize inbox */
    newproc->inbox_lock = 0;
    newproc->blocked_for_msg = 0;
    linkedlist_init(&(newproc->inbox));

    /* children */
    newproc->blocked_for_child = 0;

    /* not blocked */
    newproc->blocked = 0;

    /* exit status: */
    newproc->terminated = 0;
    newproc->status = 0;

    /* add the new process to the list of processes: */
    linkedlist_addlast((linkedlist*)&proclist, (linknode*)&(newproc->plist));

    /* add to scheduler's queue: */
    linkedlist_addlast((linkedlist*)&q_ready, (linknode*)&(newproc->sched));

    /* return to the parent. */
    return newproc->pid;
}
