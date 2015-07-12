/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> Filesystem: stat() routine.                      | |
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
#include <sys/fs.h>
#include <sys/scheduler.h>

/***************************************************************************/
/*                             inode_stat()                                */
/***************************************************************************/

void inode_stat(inode_t *inode, stat_t *buf) {

    buf->ino     = inode->ino;
    buf->mode    = inode->mode;
    buf->devid   = inode->devid;
    buf->size    = inode->size;
    buf->blksize = inode->blksize;
    buf->blocks  = inode->blocks;

}

/***************************************************************************/
/*                                stat()                                   */
/***************************************************************************/

int32_t stat(char *pathname, stat_t *buf) {

    /* stat() system call. */
    namei_t namei_data;
    int32_t err;

    /* get the inode structure: */
    if (err = namei(NULL, pathname, &namei_data))
        return -err;

    /* do the stat: */
    inode_stat(namei_data.inode, buf);

    /* drop the inode: */
    iput(namei_data.inode);

    /* free the memory used by namei returned path: */
    kfree(namei_data.path);

    /* done: */
    return ESUCCESS;

}

/***************************************************************************/
/*                               fstat()                                   */
/***************************************************************************/

int32_t fstat(int32_t fd, stat_t *buf) {

    /* fd must be a valid open descriptor: */
    if (fd < 0 || fd >= FD_MAX || curproc->file[fd] == NULL)
        return -EBADF;

    /* do the stat: */
    inode_stat(curproc->file[fd]->inode, buf);

    /* done: */
    return ESUCCESS;
}

/***************************************************************************/
/*                               statfs()                                  */
/***************************************************************************/

int32_t statfs(char *pathname, statfs_t *buf) {
    /* not yet.. */
}

/***************************************************************************/
/*                              fstatfs()                                  */
/***************************************************************************/

int32_t fstatfs(int32_t fd, statfs_t *buf) {
    /* not yet.. */
}
