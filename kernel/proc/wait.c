/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> procman: wait() and waitpid() syscalls.          | |
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

/***************************************************************************/
/*                                waitpid()                                */
/***************************************************************************/

int32_t waitpid(int32_t pid, int32_t *status) {

    /* loop on process linked list until find the process with the "pid". */
    pd_t *prev  = NULL;
    pd_t *child = proclist.first;

    while (child != NULL && child->proc->pid != pid) {
        prev = child;
        child = child->next;
    }

    if (child == NULL)
        return -1; /* pid is invalid */

    while(!(child->proc->terminated)) {
        /* wait until the child exits. */
        curproc->blocked_for_child = pid;
        block();
    };

    /* return status */
    *status = child->proc->status;

    /* remove process descriptor from the list: */
    linkedlist_remove((linkedlist*)&proclist,(linknode*)child,(linknode*)prev);

    /* unallocated the process structure. */
    kfree(child->proc->kstack);
    kfree(child->proc);

    /* return */
    return pid;
}

/***************************************************************************/
/*                                 wait()                                  */
/***************************************************************************/

int32_t wait() {

}
