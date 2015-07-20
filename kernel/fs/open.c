/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Filesystem: File access routines.                | |
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
#include <lib/string.h>

/***************************************************************************/
/*                             file_open()                                 */
/***************************************************************************/

int32_t file_open(char *path, int32_t flags, file_t **ret) {

    int32_t err;
    namei_t namei_data;
    file_t *file;

    /* allocate file structure */
    file = kmalloc(sizeof(file_t));
    if (!file) {
        return ENOMEM;
    }

    /* convert path to namei structure: */
    err = namei(NULL, path, &namei_data);
    if (err) {
        kfree(file);
        return err;
    }

    /* initialize file structure: */
    file->fcount = 1;
    file->path = namei_data.path;
    file->mp = namei_data.mp;
    file->inode = namei_data.inode;
    file->pos   = 0;

    /* do file system specific operations: */
    err = file->inode->sb->fsdriver->open(file);

    /* return: */
    if (err) {
        iput(file->inode);
        kfree(file->path);
        kfree(file);
    } else {
        *ret = file;
    }
    return err;

}

/***************************************************************************/
/*                            file_reopen()                                */
/***************************************************************************/

int32_t file_reopen(file_t *afile, file_t **ret) {

    int32_t err;
    file_t *file;

    /* allocate file structure */
    file = kmalloc(sizeof(file_t));
    if (!file) {
        return ENOMEM;
    }

    /* allocate file path string */
    file->path = kmalloc(strlen(afile->path)+1);
    if (!(file->path)) {
        kfree(file);
        return ENOMEM;
    }

    /* initialize file structure: */
    file->fcount = 1;
    strcpy(file->path, afile->path);
    file->mp    = afile->mp;
    file->inode = (inode_t *) iget(afile->mp->sb, afile->inode->ino);
    file->pos   = 0;

    /* do file system specific operations: */
    err = file->inode->sb->fsdriver->open(file);

    /* return: */
    if (err) {
        iput(file->inode);
        kfree(file->path);
        kfree(file);
    } else {
        *ret = file;
    }
    return err;

}

/***************************************************************************/
/*                                open()                                   */
/***************************************************************************/

int32_t open(char *path, int32_t flags) {

    /* the famous open() system call ^_^ */
    int32_t fd, err;

    /* look for a free fd in the current process structure: */
    for (fd = 0; fd < FD_MAX && curproc->file[fd]; fd++);

    /* no free descriptor? */
    if (fd == FD_MAX)
        return -EMFILE;

    /* do the actual opening: */
    err = file_open(path, flags, &curproc->file[fd]);

    /* error? */
    if (err) {
        curproc->file[fd] = NULL;
        return -err;
    }

    /* success: */
    return fd;

}

/***************************************************************************/
/*                             file_close()                                */
/***************************************************************************/

int32_t file_close(file_t *file) {

    int32_t err;

    /* decrease count of references: */
    file->fcount--;

    /* still referenced? */
    if (file->fcount)
        return ESUCCESS;

    /* release: */
    err = file->inode->sb->fsdriver->release(file);
    if (err)
        return err;

    /* deallocate: */
    iput(file->inode);
    kfree(file->path);
    kfree(file);

    /* return: */
    return ESUCCESS;

}

/***************************************************************************/
/*                                close()                                  */
/***************************************************************************/

int32_t close(int32_t fd) {

    int32_t err;

    /* fd must be a valid open descriptor: */
    if (fd < 0 || fd >= FD_MAX || curproc->file[fd] == NULL)
        return -EBADF;

    /* close the descriptor: */
    err = file_close(curproc->file[fd]);
    if (err) {
        return -err;
    }

    /* zeroise the pointer: */
    curproc->file[fd] = NULL;

    /* done: */
    return ESUCCESS;

}

/***************************************************************************/
/*                               chdir()                                   */
/***************************************************************************/

int32_t chdir(char *dir) {

    file_t *newdir;
    int32_t err;

    /* open the new directory: */
    err = file_open(dir, 0, &newdir);
    if (err)
        return -err;

    /* must be directory: */
    if ((newdir->inode->mode & FT_MASK) != FT_DIR) {
        file_close(newdir);
        return -ENOTDIR;
    }

    /* close current working directory: */
    file_close(curproc->cwd);

    /* set the new directory: */
    curproc->cwd = newdir;

    /* done: */
    return ESUCCESS;

}

/***************************************************************************/
/*                              getcwd()                                   */
/***************************************************************************/

int32_t getcwd(char *buf, int32_t size) {

    /* size is currently ignored. */
    strcpy(buf, curproc->cwd->path);
    return strlen(curproc->cwd->path);

}

/***************************************************************************/
/*                          inode_truncate()                               */
/***************************************************************************/

int32_t inode_truncate(inode_t *inode, pos_t length) {

    return inode->sb->fsdriver->truncate(inode, length);

}

/***************************************************************************/
/*                             truncate()                                  */
/***************************************************************************/

int32_t truncate(char *pathname, pos_t length) {

    /* truncate() system call. */
    namei_t namei_data;
    int32_t err;

    /* get the inode structure: */
    if (err = namei(NULL, pathname, &namei_data))
        return -err;

    /* do the truncate: */
    err = inode_truncate(namei_data.inode, length);

    /* drop the inode: */
    iput(namei_data.inode);

    /* free the memory used by namei returned path: */
    kfree(namei_data.path);

    /* done: */
    if (err) {
        return -err;
    } else {
        return ESUCCESS;
    }

}

/***************************************************************************/
/*                            ftruncate()                                  */
/***************************************************************************/

int32_t ftruncate(int32_t fd, pos_t length) {

    int32_t err;

    /* fd must be a valid open descriptor: */
    if (fd < 0 || fd >= FD_MAX || curproc->file[fd] == NULL)
        return -EBADF;

    /* do the truncate: */
    err = inode_truncate(curproc->file[fd]->inode, length);

    /* done: */
    if (err) {
        return -err;
    } else {
        return ESUCCESS;
    }

}
