/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Filesystem header.                               | |
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

#ifndef FS_H
#define FS_H

#include <arch/type.h>
#include <sys/device.h>
#include <lib/linkedlist.h>
#include <lib/string.h>
#include <sys/syscall.h>
#include <sys/error.h>
#include <sys/mm.h>
#include <fs/tmpfs.h>
#include <fs/diskfs.h>

/* Inode number: */
typedef uint32_t ino_t;

/* File mode: */
#define FT_REGULAR      0x0000
#define FT_DIR          0x1000
#define FT_SPECIAL      0x2000
#define FT_MASK         0xF000
typedef uint32_t mode_t;

/* position control: */
#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2
typedef int64_t pos_t;

/* file descriptors: */
#define FD_MAX          1024 /* maximum open files for a process */

/* file name: */
#define FILENAME_MAX    4096 /* maximum file name */

/* filesystem driver flags: */
#define FSD_REQDEV      0x01

/* root of the filesystem: */
extern struct vfsmount *vfsroot;

/* Supported Filesystems: */
extern struct fsd tmpfs_t;
extern struct fsd devfs_t;
extern struct fsd sysfs_t;
extern struct fsd diskfs_t;
extern struct fsd *fsdrivers[];
#define FSDRIVER_COUNT  (sizeof(fsdrivers)/sizeof(fsd_t*))

/* stat structure: */
typedef struct stat {
    ino_t ino;        /* inode number.    */
    mode_t mode;
    uint32_t devid;   /* if special file. */
    uint64_t size;
    uint64_t blksize;
    uint64_t blocks;
} stat_t;

/* statfs structure: */
typedef struct statfs {
    uint32_t tmp;
} statfs_t;

/* Directory Entries: */
typedef struct dirent {
    ino_t ino;
    char name[FILENAME_MAX+1];
} dirent_t;

/* VFS Mount Point: */
typedef struct vfsmount {
    struct vfsmount *vfsparent; /* parent mountpoint. */
    struct inode *mp_inode;     /* inode structure of the mount
                                 * point <on the parent fs>
                                 */
    struct super_block *sb;     /* vfs superblock. */
} vfsmount_t;

/* VFS super block: */
typedef struct super_block {
    struct fsd *fsdriver; /* a pointer to filesystem driver struct. */
    device_t *dev;        /* disk hardware device.                  */
    ino_t root_ino;       /* root inode.                            */
    int32_t icount;       /* number of inode handles.               */
    int32_t mounts;       /* mounts...                              */
    int32_t blksize;
    void *disksb;         /* a copy of disk superblock.             */
} super_block_t;

/* VFS inode: */
typedef struct inode {
    /* for container linked list: */
    struct inode *next;

    /* identity: */
    super_block_t *sb; /* filesystem to which this inode belongs. */
    ino_t ino;         /* inode number.                           */

    /* run time info: */
    int32_t icount;    /* count of handlers.                      */
    device_t *dev;

    /* sub mount point? */
    vfsmount_t *submount;

    /* info about the inode on disk: */
    int32_t ref;  /* count of references on disk. */
    mode_t mode;
    uint32_t devid;
    uint32_t blksize;
    uint64_t size;
    uint64_t blocks;

    /* file mapping (shared memory areas) */
    _linkedlist(file_mem_t) sma;

    /* filesystem specific information: */
    union {
        diskfs_inode_info_t diskfs;
        tmpfs_inode_info_t tmpfs;
    } info;
} inode_t;

/* namei data: */
typedef struct namei {
    /* full path: */
    char *path;

    /* matching vfs inode: */
    inode_t *inode;

    /* mountpoint: */
    struct vfsmount *mp;
} namei_t;

/* VFS file: */
typedef struct file {
    /* references: */
    int32_t fcount; /* affected by dup(), dup2(), and fork(). */

    /* file name: */
    char *path;

    /* mount point of path: */
    struct vfsmount *mp;

    /* the matching vfs inode structure: */
    inode_t *inode;

    /* position: */
    pos_t pos;

    /* filesystem specific information: */
    union {
        diskfs_file_info_t diskfs;
        tmpfs_file_info_t tmpfs;
    } info;
} file_t;

/* Filesystem Driver Structure: */
typedef struct fsd {
    /* name: */
    char *alias;

    /* flags: */
    int32_t flags;

    /* super block operations: */
    super_block_t *(*read_super)(device_t *dev);
    int32_t (*write_super)(super_block_t *sb);
    int32_t (*put_super)(super_block_t *sb);
    int32_t (*read_inode)(inode_t *inode);
    int32_t (*update_inode)(inode_t *inode);
    int32_t (*put_inode)(inode_t *inode);

    /* inode operations: */
    int32_t (*lookup)(inode_t *dir, char *name, inode_t **ret);
    int32_t (*mknod)(inode_t *dir, char *name, int32_t mode, int32_t devid);
    int32_t (*link)(inode_t *inode, inode_t *dir, char *name);
    int32_t (*unlink)(inode_t *dir, char *name);
    int32_t (*mkdir)(inode_t *dir, char *name, int32_t mode);
    int32_t (*rmdir)(inode_t *dir, char *name);
    int32_t (*truncate)(inode_t *inode, pos_t length);

    /* file operations */
    int32_t (*open)(file_t *file);
    int32_t (*release)(file_t *file);
    int32_t (*read)(file_t *file, void *buf, int32_t size);
    int32_t (*write)(file_t *file, void *buf, int32_t size);
    int32_t (*seek)(file_t *file, pos_t newpos);
    int32_t (*readdir)(file_t *dir, dirent_t *dirent);
    int32_t (*ioctl)(file_t *file, int32_t cmd, void *arg);
} fsd_t;

#endif
