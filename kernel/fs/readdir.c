/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> Filesystem: readdir() routine.                   | |
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
/*                            file_readdir()                               */
/***************************************************************************/

int32_t file_readdir(file_t *file, dirent_t *dirp) {

    /* file must be directory: */
    if ((file->inode->mode & FT_MASK) != FT_DIR)
        return 0;

    /* give control to filesystem driver. */
    return file->inode->sb->fsdriver->readdir(file, dirp);

}

/***************************************************************************/
/*                               readdir()                                 */
/***************************************************************************/

int32_t readdir(int32_t fd, dirent_t *dirp) {

    /* fd must be a valid open descriptor: */
    if (fd < 0 || fd >= FD_MAX || curproc->file[fd] == NULL)
        return EBADF;

    /* do the read: */
    return -file_readdir(curproc->file[fd], dirp);

}
