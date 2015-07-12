/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> Filesystem: File read & write routines.          | |
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
/*                             file_read()                                 */
/***************************************************************************/

int32_t file_read(file_t *file, void *buf, size_t count, ssize_t *done) {

    int32_t err;
    pos_t oldpos = file->pos;

    switch(file->inode->mode & FT_MASK) {
        case FT_REGULAR:
        err = file->inode->sb->fsdriver->read(file, buf, count);
        *done = (ssize_t) (file->pos - oldpos);
        break;

        case FT_DIR:
        err = EISDIR;
        *done = (ssize_t) (file->pos - oldpos);
        break;

        case FT_SPECIAL:
        dev_read(file->inode->dev, file->pos, count, buf);
        err = ESUCCESS;
        file->pos += count;
        *done = count;
        break;
    }

    return err;

}

/***************************************************************************/
/*                                read()                                   */
/***************************************************************************/

int32_t read(int32_t fd, void *buf, size_t count) {

    int32_t err;
    ssize_t done;

    /* fd must be a valid open descriptor: */
    if (fd < 0 || fd >= FD_MAX || curproc->file[fd] == NULL)
        return -EBADF;

    /* do the read: */
    err = file_read(curproc->file[fd], buf, count, &done);

    /* return result: */
    if (!err) {
        return done;
    } else {
        return -err;
    }

}

/***************************************************************************/
/*                             file_write()                                */
/***************************************************************************/

int32_t file_write(file_t *file, void *buf, size_t count, ssize_t *done) {

    int32_t err;
    pos_t oldpos = file->pos;

    switch(file->inode->mode & FT_MASK) {
        case FT_REGULAR:
        err = file->inode->sb->fsdriver->write(file, buf, count);
        break;

        case FT_DIR:
        err = EISDIR;
        break;

        case FT_SPECIAL:
        dev_write(file->inode->dev, file->pos, count, buf);
        err = ESUCCESS;
        file->pos += count;
        break;
    }

    *done = file->pos - oldpos;
    return err;

}

/***************************************************************************/
/*                                write()                                  */
/***************************************************************************/

int32_t write(int32_t fd, void *buf, size_t count) {

    int32_t err;
    ssize_t done;

    /* fd must be a valid open descriptor: */
    if (fd < 0 || fd >= FD_MAX || curproc->file[fd] == NULL)
        return -EBADF;

    /* do the write: */
    err = file_write(curproc->file[fd], buf, count, &done);

    /* return result: */
    if (!err) {
        return done;
    } else {
        return -err;
    }

}

/***************************************************************************/
/*                             file_seek()                                 */
/***************************************************************************/

void file_seek(file_t *file, pos_t offset, pos_t *result, int32_t whence) {

    pos_t newpos;
    pos_t file_size = file->inode->size;

    switch(whence) {
        case SEEK_SET:
        newpos = offset;
        break;

        case SEEK_CUR:
        newpos = file->pos + offset;
        break;

        case SEEK_END:
        newpos = file_size + offset;
        break;

        default:
        newpos = file->pos;
    }

    switch(file->inode->mode & FT_MASK) {
        case FT_REGULAR:
        file->inode->sb->fsdriver->seek(file, newpos);
        break;

        case FT_DIR:
        break;

        case FT_SPECIAL:
        file->pos = newpos;
        break;
    }

    if (result)
        *result = file->pos;

}

/***************************************************************************/
/*                                seek()                                   */
/***************************************************************************/

int32_t seek(int32_t fd, pos_t offset, pos_t *result, int32_t whence) {

    /* fd must be a valid open descriptor: */
    if (fd < 0 || fd >= FD_MAX || curproc->file[fd] == NULL)
        return -EBADF;

    /* do the seek: */
    file_seek(curproc->file[fd], offset, result, whence);

    /* done: */
    return 0;

}
