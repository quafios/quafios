/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Format Utility.                             | |
 *        | |  -> diskfs format for GNU/Linux.                     | |
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

#include <sys/stat.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <limits.h>
#include <dirent.h>
#include <unistd.h>

/* typedef char                    int8_t; */
typedef short                   int16_t;
typedef int                     int32_t;
/* __extension__ typedef long long int64_t; */

typedef unsigned char                    uint8_t;
typedef unsigned short                   uint16_t;
typedef unsigned int                     uint32_t;
__extension__ typedef unsigned long long uint64_t;

#include "../kernel/include/fs/diskfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BLOCK_SIZE    1024 /* 1KB */

__extension__ typedef uint64_t disk_size_t;
__extension__ typedef uint64_t disk_off_t;

int32_t diskfd;

/***************************************************************************/
/*                                Disk I/O                                 */
/***************************************************************************/

int32_t disk_open(char *devfile) {
    errno = 0;
    diskfd = open(devfile, O_RDWR);
    return errno;
}

disk_size_t disk_size() {
    struct stat buf;
    fstat(diskfd, &buf);
    return buf.st_size;
}

int32_t disk_read(disk_off_t off, void *buf, int32_t count) {
    if (lseek(diskfd, off, SEEK_SET) != off)
        return 0;
    return read(diskfd, buf, count);
}

int32_t disk_write(disk_off_t off, void *buf, int32_t count) {
    if (lseek(diskfd, off, SEEK_SET) != off)
        return 0;
    return write(diskfd, buf, count);
}

int32_t disk_close() {
    return close(diskfd);
}

/***************************************************************************/
/*                              Cluster I/O                                */
/***************************************************************************/

int32_t read_cluster(diskfs_sb_t *sb, diskfs_pos_t clus, void *buf) {
    disk_off_t offset = 512 /* boot sect */ + clus*sb->block_size;
    return disk_read(offset, buf, sb->block_size)/sb->block_size;
}

int32_t write_cluster(diskfs_sb_t *sb, diskfs_pos_t clus, void *buf) {
    disk_off_t offset = 512 /* boot sect */ + clus*sb->block_size;
    return disk_write(offset, buf, sb->block_size)/sb->block_size;
}

/***************************************************************************/
/*                                 Format                                  */
/***************************************************************************/

diskfs_sb_t *format() {
    /* format the disk & install the filesystem. */
    uint32_t i;
    uint8_t *cluster;
    diskfs_sb_t *sb;
    uint32_t inode_map_bytes;
    diskfs_blk_t alldata;
    diskfs_pos_t data_map_bytes;

    /* Allocate Superblock: */
    cluster = malloc(BLOCK_SIZE);
    for (i = 0; i < BLOCK_SIZE; i++)
        cluster[i] = 0;
    sb = (diskfs_sb_t *) cluster;

    /* initialize the superblock: */
    sb->magic = QUAFS_MAGIC;
    sb->revision = QUAFS_REVISION;
    sb->creator_os = QUAFS_LINUX;

    /* Disk Size & Blocks: */
    sb->disk_size = disk_size();
    sb->block_size = BLOCK_SIZE;
    if ((sb->disk_size-512)/sb->block_size > 0xFFFFFFFF) {
        printf("Disk is very big!\n");
        free(sb);
        return NULL;
    }
    sb->total_blocks = (sb->disk_size-512)/sb->block_size;

    /* Inodes Map & Region:
     * inodes size is 128B.
     * decision equation:
     * count of inode blocks = total blocks/32.
     * thus: inodes count = (inode_blocks)*(block_size/inode_size)
     */
    sb->inode_blocks = sb->total_blocks/32;
    sb->total_inodes = (sb->inode_blocks)*
                       (sb->block_size/sizeof(diskfs_inode_t));
    sb->inode_map_start = 1; /* skip superblock */
    inode_map_bytes = (sb->total_inodes/8) + (sb->total_inodes%8 ? 1 : 0);
    sb->inode_map_blocks = (inode_map_bytes/sb->block_size) +
                           (inode_map_bytes%sb->block_size ? 1 : 0);
    sb->next_free_inode = DISKFS_ROOT_INO;
    sb->free_inodes = sb->total_inodes - DISKFS_ROOT_INO;

    /* Data Map & Region:
     * Remaining blocks are for data & data map.
     * assuming that data blocks count = x, then:
     * x*block_size + x/8 + 1 = (remaining blocks)*block_size
     * x*(8*block_size+1) = (remaining*block_size-1)*8
     * then x = (remaining*block_size-1)*8/(8*block_size+1)
     */
    sb->data_map_start = sb->inode_map_start + sb->inode_map_blocks;
    alldata = sb->total_blocks - sb->data_map_start - sb->inode_blocks;
    sb->data_blocks = ((alldata*sb->block_size-1)*8) /
                      (8*sb->block_size+1);
    data_map_bytes = (sb->data_blocks/8) + (sb->data_blocks%8 ? 1 : 0);
    sb->data_map_blocks  = (data_map_bytes/sb->block_size) +
                           (data_map_bytes%sb->block_size ? 1 : 0);
    sb->free_data_blocks = sb->data_blocks;
    sb->next_free_block  = 0;

    /* Inode & Data Region: */
    sb->inode_start = sb->data_map_start + sb->data_map_blocks;
    sb->data_start  = sb->inode_start + sb->inode_blocks;

    /* Set Identifiers: */
    strcpy(sb->name, "Quafios Disk");
    strcpy(sb->uuid, "0123456789ABCDEF");

    /* Make sure that my calculations are correct: */
    if (sb->data_map_blocks + sb->data_blocks + sb->inode_map_blocks
        + sb->inode_blocks > sb->total_blocks) {
        printf("mkimage.c: Bug in format(), calculated total ");
        printf(           "number of filesystem blocks\n");
        printf("           is larger than actual count of blocks.\n");
        printf("           Please report this bug to ");
        printf(           "iocoder@aol.com\n");
        free(sb);
        return NULL;
    }

    /* Done: */
    return sb;
}

/***************************************************************************/
/*                           Filesystem Operations                         */
/***************************************************************************/

void update_inode(diskfs_sb_t *sb, diskfs_ino_t ino, diskfs_inode_t *inode) {

    /* calculate the location of the cluster that
     * contains the desired inode.
     */
    uint32_t inodes_per_cluster = sb->block_size/sizeof(diskfs_inode_t);
    diskfs_blk_t cluster = ino/inodes_per_cluster + sb->inode_start;

    /* read the cluster into memory: */
    diskfs_inode_t *buf = malloc(sb->block_size);
    read_cluster(sb, cluster, (void *) buf);

    /* do the update! */
    buf[ino%inodes_per_cluster] = *inode;

    /* write the new updates to disk: */
    write_cluster(sb, cluster, (void *) buf);

    /* i think we are done? */
    free(buf);

}

diskfs_blk_t get_block(diskfs_sb_t *sb) {

    diskfs_blk_t blk;
    if (sb->free_data_blocks == 0)
        return 0;
    sb->free_data_blocks--;
    blk = sb->next_free_block++;
    if (sb->next_free_block == sb->data_blocks)
        sb->next_free_block = 0;
    return blk + sb->data_start;

}

diskfs_blk_t alloc_file_block(diskfs_sb_t *sb,
                              diskfs_ino_t ino,
                              diskfs_inode_t *inode,
                              diskfs_blk_t blk_off) {

    /* allocate a block for the file
     * referenced by "inode". the block
     * is allocated at the offset "blk_off"
     * from the beginning of the file.
     * if there is already an allocated block,
     * don't allocate another one.
     */

    int32_t ptrs_per_block = sb->block_size/sizeof(diskfs_blk_t);
    diskfs_lvl_t lvl = diskfs_blk_to_lvl(sb, blk_off);
    uint8_t *tmp = malloc(sb->block_size);
    int32_t init = 0, i;
    diskfs_blk_t  blk;
    diskfs_blk_t *ptr;

    for (i = 0; i <= lvl.level; i++) {

        /* load the table into memory: */
        if (!i) {
            ptr = inode->ptr;
        } else {
            read_cluster(sb, blk, tmp);
            ptr = (diskfs_blk_t *) tmp;
        }

        /* initialize the table? */
        if (init) {
            int32_t j;
            while(j < ptrs_per_block)
                ptr[j++] = 0;
            init = 0;
        }

        /* allocate a pointer for next level (if there isn't): */
        if (ptr[lvl.ptr[i]]) {

            blk = ptr[lvl.ptr[i]]; /* next level; */

        } else {

            /* allocate and update: */
            ptr[lvl.ptr[i]] = get_block(sb);

            /* the new allocated page need to be initialized: */
            init = 1;

            /* write updates to disk */
            if (!i)
                update_inode(sb, ino, inode);
            else
                write_cluster(sb, blk, tmp);

            /* set blk to next level; */
            blk = ptr[lvl.ptr[i]];

            /* NULL? */
            if (!blk)
                return 0;

        }
    }

    /* done */
    free(tmp);
    return blk;

}

void write_bitmap(diskfs_sb_t *sb,
                  diskfs_blk_t start,
                  diskfs_blk_t totalBits,
                  diskfs_blk_t freeBits) {

    uint8_t *buf = malloc(sb->block_size);
    int32_t bitsPerBlock = sb->block_size*8;
    diskfs_blk_t totalBlocks = totalBits/bitsPerBlock +
                               (totalBits%bitsPerBlock ? 1 : 0);
    diskfs_blk_t usedBits = totalBits - freeBits;
    diskfs_blk_t i, j, cur = 0;

    for(i = start; i < start+totalBlocks; i++) {

        for(j = 0; j < bitsPerBlock; j++)
            if (cur++ < usedBits)
                buf[j/8] |= 1<<(j%8);
            else
                buf[j/8] &= ~(1<<(j%8));

        /* write the block to disk: */
        write_cluster(sb, i, buf);
    }

    free(buf);

}

/***************************************************************************/
/*                                Copy Files                               */
/***************************************************************************/

diskfs_ino_t cp(diskfs_sb_t *sb, diskfs_ino_t parent, char *name) {

    diskfs_ino_t   ino;
    diskfs_inode_t inode;

    struct stat stat;
    int32_t fd;

    off_t rem;
    int32_t  i;

    diskfs_blk_t blk;
    diskfs_blk_t disk_blk;
    uint8_t *block = malloc(sb->block_size);

    DIR *dirpp;
    diskfs_dirent_t *dir;

    /* Open the file:  */
    /* --------------- */
    /* do the open */
    fd = open(name, O_RDONLY);

    /* stat the file: */
    fstat(fd, &stat);

    /* Reserve an inode:  */
    /* ------------------ */
    if (sb->free_inodes == 0) {
        close(fd);
        return 0;
    }
    sb->free_inodes--;
    ino = sb->next_free_inode++;
    if (sb->next_free_inode == sb->total_inodes)
        sb->next_free_inode = 0;

    /* Initialize the inode:  */
    /* ---------------------- */
    DISKFS_CLEAN_INODE(inode);
    inode.ref  = 1;
    if (S_ISREG(stat.st_mode)) {
        inode.mode = DISKFS_FT_REG;
    } else if (S_ISDIR(stat.st_mode)) {
        inode.mode = DISKFS_FT_DIR;
    } else {
        return 0;
    }
    inode.size   = 0;
    inode.blocks = 0;
    inode.devid  = 0;
    for(i = 0; i < DISKFS_PTRS; i++)
        inode.ptr[i] = 0;
    update_inode(sb, ino, &inode);

    /* Regular File? Copy data to disk:  */
    /* --------------------------------- */
    rem = stat.st_size;
    i = 0;
    while (S_ISREG(stat.st_mode) && rem) {

        int32_t readsize = rem > sb->block_size ? sb->block_size : rem;

        /* allocate block: */
        disk_blk = alloc_file_block(sb, ino, &inode, i++);
        if (!disk_blk) {
            /* no enough space */
            free(block);
            return ino;
        }

        /* update the inode: */
        inode.size += readsize;
        inode.blocks++;
        update_inode(sb, ino, &inode);

        /* read next block into memory: */
        read(fd, block, readsize); /* linux. */
        rem -= readsize;
        while (readsize < sb->block_size)
            block[readsize++] = 0; /* zero padding. */

        /* write the block to disk: */
        write_cluster(sb, disk_blk, block);

    }

    /* Directory? create dir entries:  */
    /* ------------------------------- */
    if (S_ISDIR(stat.st_mode))
        dirpp = fdopendir(fd);

    /* directory block is actually an array of directory entries: */
    dir = (diskfs_dirent_t *) block;

    /* counters: */
    blk = 0; /* counter for blocks */
    i = 0; /* counter for entires in every block. */

    /* allocate a block for the dir entry: */
    disk_blk = alloc_file_block(sb, ino, &inode, blk);
    if (!disk_blk) {
        free(block);
        return ino;
    }

    /* zeroise the block */
    while(i < sb->block_size)
        block[i++] = 0;
    i = 0;

    /* insert the "." and ".." */
    dir[i].ino = ino;
    strcpy(dir[i++].name, ".");
    dir[i].ino = parent;
    strcpy(dir[i++].name, "..");

    while(S_ISDIR(stat.st_mode)) {

        /* get next dir entry: */
        struct dirent *dirp = readdir(dirpp);

        /* ignore "." & ".." */
        if (dirp &&
            (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")))
            continue;

        /* TODO: ignore long names. */

        /* end of block? */
        if (i == DIRENT_COUNT(sb->block_size) || (!dirp)) {

            /* write the block to disk. */
            write_cluster(sb, disk_blk, block);

            /* done? */
            if (!dirp)
                break;

            /* counters: */
            blk++; /* counter for blocks                  */
            i = 0; /* counter for entires in every block. */

            /* allocate a block for the dir entry: */
            disk_blk = alloc_file_block(sb, ino, &inode, blk);
            if (!disk_blk) {
                free(block);
                return ino;
            }

            /* zeroise the block */
            while(i < sb->block_size)
                block[i++] = 0;
            i = 0;
        }

        /* a new entry... create a new inode in the disk: */
        fchdir(fd);
        strcpy(dir[i].name, dirp->d_name);
        dir[i].ino = cp(sb, ino, dirp->d_name);
        i++;
    }

    /* done */
    if (S_ISDIR(stat.st_mode))
        closedir(dirpp);
    free(block);
    return ino;
}

void summary(diskfs_sb_t *sb) {
    /* Print Summary: */
    printf("Summary of the filesystem:\n");
    printf("Block Size:     %-8d byte(s).\n", sb->block_size);
    printf("Total Blocks:   %-8d block(s).\n", sb->total_blocks);
    printf("Super Block:    %-8d %-8d block(s).\n", 0, 1);
    printf("Inode Map:      %-8d %-8d block(s).\n",
           sb->inode_map_start, sb->inode_map_blocks);
    printf("Data Map:       %-8d %-8d block(s).\n",
           sb->data_map_start, sb->data_map_blocks);
    printf("Inode Region:   %-8d %-8d block(s).\n",
           sb->inode_start, sb->inode_blocks);
    printf("Data Region:    %-8d %-8d block(s).\n",
           sb->data_start, sb->data_blocks);
    printf("Total inodes:   %-8d inode(s).\n", sb->total_inodes);

}

/***************************************************************************/
/*                                  Main                                   */
/***************************************************************************/

int main(int argc, char *argv[]) {

    /* Make Quafios File System (on GNU/Linux). */

    int32_t err;
    diskfs_sb_t *sb;

    if (argc < 3) {
        printf("Usage: mkimage DIR IMAGE.\n");
        return -1;
    }

    /* open le disk:argv[2] */
    if (err = disk_open(argv[2])) {
        printf("Error %x while opening disk %s\n", err, argv[2]);
        return err;
    }

    /* create the superblock: */
    sb = format();

    /* copy files & directories to disk: */
    cp(sb, DISKFS_ROOT_INO, argv[1]);

    /* write the superblock: */
    write_cluster(sb, 0, (void *) sb);

    /* write the inode bitmap: */
    write_bitmap(sb,
                 sb->inode_map_start,
                 sb->total_inodes,
                 sb->free_inodes);

    /* write the data bitmap: */
    write_bitmap(sb,
                 sb->data_map_start,
                 sb->data_blocks,
                 sb->free_data_blocks);

    /* print summary: */
    summary(sb);

    return 0;

}
