/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Filesystem: File control routines.               | |
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
#include <sys/mm.h>
#include <sys/fs.h>
#include <sys/scheduler.h>

/***************************************************************************/
/*                                 dup()                                   */
/***************************************************************************/

int32_t dup(int32_t oldfd) {

    int32_t newfd;

    /* oldfd must be a valid descriptor */
    if (oldfd < 0 || oldfd >= FD_MAX || curproc->file[oldfd] == NULL)
        return -EBADF;

    /* look for a new free descriptor: */
    for (newfd = 0; newfd < FD_MAX && curproc->file[newfd]; newfd++);

    /* no free descriptor? */
    if (newfd == FD_MAX)
        return -EMFILE;

    /* increase references: */
    curproc->file[oldfd]->fcount++;

    /* duplicate! */
    curproc->file[newfd] = curproc->file[oldfd];

    /* done: */
    return ESUCCESS;

}

/***************************************************************************/
/*                                 dup2()                                  */
/***************************************************************************/

int32_t dup2(int32_t oldfd, int32_t newfd) {

    int32_t err;

    /* oldfd must be a valid open descriptor: */
    if (oldfd < 0 || oldfd >= FD_MAX || curproc->file[oldfd] == NULL)
        return -EBADF;

    /* newfd must be a valid descriptor: */
    if (oldfd < 0 || newfd >= FD_MAX)
        return -EBADF;

    /* if newfd is open, close it: */
    if (curproc->file[newfd]) {
        if (err = file_close(curproc->file[newfd]))
            return -err;
        curproc->file[newfd] = NULL;
    }

    /* increase references: */
    curproc->file[oldfd]->fcount++;

    /* duplicate! */
    curproc->file[newfd] = curproc->file[oldfd];

    /* done: */
    return ESUCCESS;

}
