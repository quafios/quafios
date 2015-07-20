/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios C Standard Library.                         | |
 *        | |  -> API: Filesystem header.                          | |
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

#ifndef __API_FS_H
#define __API_FS_H

#include <sys/fs.h>    /* kernel filesystem structures. */
#include <sys/error.h> /* kernel error codes.           */

int mount(char *devfile, char *mntpoint, char *fstype,
          unsigned int flags, void *data);
int umount(char *target);
int mknod(char *pathname, mode_t mode, unsigned int devid);
int rename(char *oldpath, char *newpath);
int link(char *oldpath, char *newpath);
int unlink(char *pathname);
int mkdir(char *pathname, mode_t mode);
int rmdir(char *pathname);
int open(char *pathname, int flags);
int close(int fd);
int chdir(char *pathname);
char *getcwd(char *buf, int size);
int truncate(char *pathname, pos_t length);
int ftruncate(int fd, pos_t length);
ssize_t read(int fd, char *buf, size_t size);
ssize_t write(int fd, char *buf, size_t size);
pos_t seek(int fd, pos_t offset, int whence);
int readdir(int fd, dirent_t *dirp);
int stat(char *pathname, stat_t *buf);
int fstat(int fd, stat_t *buf);
int statfs(char *pathname, statfs_t *buf);
int fstatfs(int fd, statfs_t *buf);
int dup(int oldfd);
int dup2(int oldfd, int newfd);
int ioctl(int fd, unsigned int cmd, void *data);
int execve(char *filename, char *argv[], char *envp[]);

#endif
