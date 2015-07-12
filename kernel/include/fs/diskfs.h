/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> Disk filesystem header.                          | |
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

#ifndef QUAFIOS_DISKFS_H
#define QUAFIOS_DISKFS_H

#ifdef QUAFIOS_KERNEL
#include <arch/type.h>
#endif

#define DISKFS_SB_OFFSET        (128*1024) /* 128KB */
#define DISKFS_ROOT_INO         2

typedef uint64_t diskfs_pos_t;
typedef uint32_t diskfs_blk_t;
typedef uint32_t diskfs_ino_t;
typedef uint32_t diskfs_mode_t;

typedef struct diskfs_sb {
    #define QUAFS_MAGIC         0x19930430
    uint32_t     magic;

    #define QUAFS_REVISION      0x0000
    uint16_t     revision;

    #define QUAFS_OTHER         0
    #define QUAFS_QUAFIOS       1
    #define QUAFS_LINUX         2
    uint16_t     creator_os;

    /* Disk Parameters: */
    diskfs_pos_t disk_size;    /* in bytes */
    uint16_t     block_size;   /* in bytes */
    diskfs_blk_t total_blocks;

    /* Inode Map: */
    diskfs_blk_t inode_map_start;
    diskfs_blk_t inode_map_blocks;
    diskfs_ino_t total_inodes;
    diskfs_ino_t free_inodes;
    diskfs_ino_t next_free_inode;

    /* Data Map: */
    diskfs_blk_t data_map_start;
    diskfs_blk_t data_map_blocks;
    diskfs_blk_t free_data_blocks;
    diskfs_blk_t next_free_block;

    /* Inodes: */
    diskfs_blk_t inode_start;
    diskfs_blk_t inode_blocks;

    /* Data: */
    diskfs_blk_t data_start;
    diskfs_blk_t data_blocks;

    /* Special Inodes: */
    diskfs_ino_t boot_loader_ino;

    /* ID: */
    char         name[33];
    char         uuid[17];
} __attribute__((packed)) diskfs_sb_t;

typedef struct diskfs_inode {
    uint32_t      ref;

    #define DISKFS_FT_REG       0x0000
    #define DISKFS_FT_DIR       0x1000
    #define DISKFS_FT_DEV       0x2000
    #define DISKFS_FT_MASK      0xF000
    diskfs_mode_t mode;

    diskfs_pos_t  size;
    diskfs_blk_t  blocks;
    uint32_t      devid;

    #define DISKFS_PTRS         15
    #define DISKFS_LVL0         12
    #define DISKFS_LVL1         1
    #define DISKFS_PTR_L1       12
    #define DISKFS_LVL2         1
    #define DISKFS_PTR_L2       13
    #define DISKFS_LVL3         1
    #define DISKFS_PTR_L3       14
    diskfs_blk_t  ptr[DISKFS_PTRS];
} __attribute__ ((aligned (128))) diskfs_inode_t;

typedef struct diskfs_inode_info {
    diskfs_blk_t  ptr[DISKFS_PTRS];
} diskfs_inode_info_t;

typedef struct diskfs_file_info {
    int32_t buf_empty;    /* buffer is empty? */
    diskfs_blk_t buf_blk; /* buffered block.  */
    char *buffer;         /* buffer.          */
} diskfs_file_info_t;

#define DISKFS_CLEAN_INODE(inode) (__extension__({      \
            int __counter = 0;                          \
            while(__counter < sizeof(diskfs_inode_t))   \
                ((char *) &(inode))[__counter++] = 0;   \
        }))

typedef struct diskfs_dirent {
    diskfs_ino_t ino;
    char         name[28];
} __attribute__((packed)) __attribute__((aligned(32))) diskfs_dirent_t;

#define DISKFS_MAX_NAME   (sizeof(diskfs_dirent_t)-sizeof(diskfs_ino_t)-1)
#define DIRENT_COUNT(BLK_SIZE) (BLK_SIZE/sizeof(diskfs_dirent_t))

typedef struct diskfs_lvl {
    int32_t level;
    int32_t ptr[4];
} diskfs_lvl_t; /* used in converting a block offset into pointers. */

static __inline__ diskfs_lvl_t
diskfs_blk_to_lvl(diskfs_sb_t *sb, diskfs_blk_t blk_off) {

    /* the structure that will be returned: */
    diskfs_lvl_t lvl;

    /* i got it from http://graphics.stanford.edu/~seander/bithacks.html */
    static const int32_t bitpos[32] = {
        0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
    };

    /* initialize shifting values: */
    int32_t ptrs_per_block = sb->block_size/sizeof(diskfs_blk_t);
    diskfs_blk_t mask = ptrs_per_block-1;
    int32_t shift1 = 0;
    int32_t shift2 = bitpos[(uint32_t)(ptrs_per_block*0x077CB531U)>>27];
    int32_t shift3 = 2*shift2;

    /* example, in case of block size = 4096:
    * ptrs_per_block = 1024
    * mask   = 0x3FF
    * shift1 = 0
    * shift2 = 10
    * shift3 = 20
    */

    /* how many blocks in every level? */
    diskfs_blk_t level0_blocks = DISKFS_LVL0;
    diskfs_blk_t level1_blocks = DISKFS_LVL1*ptrs_per_block;
    diskfs_blk_t level2_blocks = DISKFS_LVL2*ptrs_per_block*ptrs_per_block;

    /* caluclate lvl values: */
    if (blk_off < level0_blocks) {

        lvl.level  = 0;
        lvl.ptr[0] = blk_off;
        lvl.ptr[1] = 0;
        lvl.ptr[2] = 0;
        lvl.ptr[3] = 0;

    } else if (blk_off < level0_blocks+level1_blocks) {

        lvl.level  = 1;
        lvl.ptr[0] = DISKFS_PTR_L1;
        lvl.ptr[1] = blk_off - level0_blocks;
        lvl.ptr[2] = 0;
        lvl.ptr[3] = 0;

    } else if (blk_off < level0_blocks+level1_blocks+level2_blocks) {

        diskfs_blk_t off = blk_off-level0_blocks-level1_blocks;

        lvl.level  = 2;
        lvl.ptr[0] = DISKFS_PTR_L2;
        lvl.ptr[1] = (off>>shift1) & mask;
        lvl.ptr[2] = (off>>shift2) & mask;
        lvl.ptr[3] = 0;

    } else {

        diskfs_blk_t off = blk_off-level0_blocks-
                            level1_blocks-level2_blocks;

        lvl.level  = 3;
        lvl.ptr[0] = DISKFS_PTR_L3;
        lvl.ptr[1] = (off>>shift1) & mask;
        lvl.ptr[2] = (off>>shift2) & mask;
        lvl.ptr[3] = (off>>shift3) & mask;

    }

    /* done: */
    return lvl;

}

#endif
