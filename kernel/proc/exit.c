/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> procman: exit() system call.                     | |
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
#include <sys/proc.h>
#include <sys/scheduler.h>
#include <sys/fs.h>

/***************************************************************************/
/*                                 exit()                                  */
/***************************************************************************/

void exit(int32_t status) {

    /* terminate current process. */
    int32_t i;

    /* init process? */
    if (curproc->pid == 1) {
        printk("%a", 0x0C);
        printk("init process just terminated!\n");
        printk("rebooting...\n");
        legacy_reboot();
    }

    /* clear memory: */
    umem_free(&(curproc->umem));

    /* close all file descriptors: */
    for(i = 0; i < FD_MAX; i++)
        close(i);

    /* close cwd */
    file_close(curproc->cwd);

    /* TODO: make all children owned by init. */

    /* unblock the parent if waiting */
    if (curproc->parent && curproc->parent->blocked_for_child==curproc->pid) {
        curproc->parent->blocked_for_child = 0;
        unblock(curproc->parent->pid);
    }

    /* set process status: */
    curproc->status = status;
    curproc->terminated = 1;

    /* ask scheduler to remove the job */
    yield();

    /* if scheduler returns, then all other jobs are blocked... */
    idle();

}
