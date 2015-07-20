/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> tmpfs header.                                    | |
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

#ifndef TMPFS_H
#define TMPFS_H

#include <arch/type.h>

#define BLOCK_SIZE  4096

typedef uint32_t tmpfs_ino_t;
typedef uint32_t tmpfs_mode_t;
typedef uint64_t tmpfs_pos_t;

/* Data block: */
typedef struct tmpfs_block {
    struct tmpfs_block *next;
    uint8_t *data;
} tmpfs_block_t;

/* Directory Entry: */
typedef struct tmpfs_dentry {
    struct tmpfs_dentry *next;
    tmpfs_ino_t inode;
    char *name;
} tmpfs_dentry_t;

/* tmpfs inode: */
typedef struct tmpfs_inode {

    /* stat: */
    int32_t ref;
    tmpfs_mode_t mode; /* mode.      */
    tmpfs_pos_t size;  /* file size. */
    uint32_t devid;

    /* data: */
    union {
        _linkedlist(tmpfs_block_t) blocks;
        _linkedlist(tmpfs_dentry_t) dentries;
    } u;

} tmpfs_inode_t;

typedef struct tmpfs_inode_info {
    int dummy;
} tmpfs_inode_info_t;

typedef struct tmpfs_file_info {
    tmpfs_block_t *curblk;
    tmpfs_dentry_t *curdent;
    uint32_t off;
    char *dummy_p;
    uint32_t dummy_i;
} tmpfs_file_info_t;

#endif
