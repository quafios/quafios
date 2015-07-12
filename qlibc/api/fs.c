/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios C Standard Library.                         | |
 *        | |  -> API: Filesystem.                                 | |
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

#include <api/fs.h>
#include <api/syscall.h>
#include <errno.h>

/**************************************************************************/
/*                               fs/super.c                               */
/**************************************************************************/

int mount(char *devfile, char *mntpoint, char *fstype,
          unsigned int flags, void *data) {
    int ret = syscall(SYS_MOUNT,devfile,mntpoint,fstype,flags,data);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

int umount(char *target) {
    int ret = syscall(SYS_UMOUNT, target);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

/**************************************************************************/
/*                               fs/namei.c                               */
/**************************************************************************/

int mknod(char *pathname, mode_t mode, unsigned int devid) {
    int ret = syscall(SYS_MKNOD, pathname, mode, devid);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

int rename(char *oldpath, char *newpath) {
    int ret = syscall(SYS_RENAME, oldpath, newpath);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

int link(char *oldpath, char *newpath) {
    int ret = syscall(SYS_LINK, oldpath, newpath);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

int unlink(char *pathname) {
    int ret = syscall(SYS_UNLINK, pathname);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

int mkdir(char *pathname, mode_t mode) {
    int ret = syscall(SYS_MKDIR, pathname, mode);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

int rmdir(char *pathname) {
    int ret = syscall(SYS_RMDIR, pathname);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

/**************************************************************************/
/*                               fs/open.c                                */
/**************************************************************************/

int open(char *pathname, int flags) {
    int ret = syscall(SYS_OPEN, pathname, flags);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return ret;
}

int close(int fd) {
    int ret = syscall(SYS_CLOSE, fd);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

int chdir(char *pathname) {
    int ret = syscall(SYS_CHDIR, pathname);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

char *getcwd(char *buf, int size) {
    int ret = syscall(SYS_GETCWD, buf, size);
    if (ret < 0) {
        errno = -ret;
        return NULL;
    }
    return buf;
}

int truncate(char *pathname, pos_t length) {
    int ret = syscall(SYS_TRUNCATE, pathname, (pos_t) length);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

int ftruncate(int fd, pos_t length) {
    int ret = syscall(SYS_FTRUNCATE, fd, (pos_t) length);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

/**************************************************************************/
/*                            fs/read_write.c                             */
/**************************************************************************/

ssize_t read(int fd, char *buf, size_t size) {
    int ret = syscall(SYS_READ, fd, buf, size);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return ret;
}

ssize_t write(int fd, char *buf, size_t size) {
    int ret = syscall(SYS_WRITE, fd, buf, size);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return ret;
}

pos_t seek(int fd, pos_t offset, int whence) {
    pos_t ret_offset;
    int ret = syscall(SYS_SEEK,fd,(pos_t)offset,&ret_offset,whence);
    if (ret < 0) {
        errno = -ret;
        return -1;
    } else {
        return ret_offset;
    }
}

/**************************************************************************/
/*                             fs/readdir.c                               */
/**************************************************************************/

int readdir(int fd, dirent_t *dirp) {
    int ret = syscall(SYS_READDIR, fd, dirp);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

/**************************************************************************/
/*                               fs/stat.c                                */
/**************************************************************************/

int stat(char *pathname, stat_t *buf) {
    int ret = syscall(SYS_STAT, pathname, buf);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

int fstat(int fd, stat_t *buf) {
    int ret = syscall(SYS_FSTAT, fd, buf);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

int statfs(char *pathname, statfs_t *buf) {
    int ret = syscall(SYS_STATFS, pathname, buf);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

int fstatfs(int fd, statfs_t *buf) {
    int ret = syscall(SYS_FSTATFS, fd, buf);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

/**************************************************************************/
/*                               fs/fcntl.c                               */
/**************************************************************************/

int dup(int oldfd) {
    int ret = syscall(SYS_DUP, oldfd);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

int dup2(int oldfd, int newfd) {
    int ret = syscall(SYS_DUP2, oldfd, newfd);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

/**************************************************************************/
/*                              fs/ioctl.c                                */
/**************************************************************************/

int ioctl(int fd, unsigned int cmd, void *data) {
    int ret = syscall(SYS_IOCTL, fd, cmd, data);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

/**************************************************************************/
/*                              fs/exec.c                                 */
/**************************************************************************/

int execve(char *filename, char *argv[], char *envp[]) {
    int ret = syscall(SYS_EXECVE, filename, argv, envp);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}
