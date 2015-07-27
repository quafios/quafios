/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Quafios disk filesystem.                         | |
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
#include <sys/fs.h>
#include <sys/mm.h>
#include <fs/diskfs.h>

/***************************************************************************/
/*                              read_super()                               */
/***************************************************************************/

super_block_t *diskfs_read_super(device_t *dev) {

    /* definitions */
    super_block_t *sb;
    diskfs_sb_t *disksb;

    /* allocate superblock: */
    sb = kmalloc(sizeof(super_block_t));
    if (!sb)
        return sb;

    /* filesystem driver: */
    sb->fsdriver = &diskfs_t;

    /* device structure: */
    sb->dev = dev;

    /* root inode number: */
    sb->root_ino = DISKFS_ROOT_INO;

    /* no inodes are open: */
    sb->icount = 0;

    /* mounts: */
    sb->mounts = 1;

    /* allocate a buffer for disk super block in memory: */
    disksb = kmalloc(512);
    if (disksb == NULL) {
        kfree(sb);
        return sb;
    }

    /* set disksb of superblock structure: */
    sb->disksb = disksb;

    /* read disksb: */
    dev_read(sb->dev, (int64_t) 1024, 512, (void *) sb->disksb);

    /* disk block size: */
    sb->blksize = disksb->block_size;

    /* return the superblock: */
    return sb;

}

/***************************************************************************/
/*                             write_super()                               */
/***************************************************************************/

int32_t diskfs_write_super(super_block_t *sb) {

    /* write 512 sector: */
    dev_write(sb->dev, (int64_t) 1024, 512, (void *) sb->disksb);

    /* done */
    return ESUCCESS;

}

/***************************************************************************/
/*                              put_super()                                */
/***************************************************************************/

int32_t diskfs_put_super(super_block_t *sb) {

    /* all file descriptors must be closed first: */
    if (sb->icount)
        return EBUSY;

    /* unallocate super block: */
    kfree(sb->disksb);
    kfree(sb);

    /* done: */
    return ESUCCESS;

}

/****************************************************************************/
/*                             read_cluster()                               */
/****************************************************************************/

int32_t diskfs_read_cluster(super_block_t *sb, pos_t clus, void *buf) {

    pos_t offset = 1024 + clus*sb->blksize;
    return dev_read(sb->dev, offset, sb->blksize, buf)/sb->blksize;

}

/****************************************************************************/
/*                            write_cluster()                               */
/****************************************************************************/

int32_t diskfs_write_cluster(super_block_t *sb, pos_t clus, void *buf) {

    pos_t offset = 1024 + clus*sb->blksize;
    return dev_write(sb->dev, offset, sb->blksize, buf)/sb->blksize;

}

/****************************************************************************/
/*                              read_inode()                                */
/****************************************************************************/

int32_t diskfs_read_inode(inode_t *inode) {

    /* definitions */
    diskfs_sb_t *disksb;
    uint32_t inodes_per_cluster;
    diskfs_blk_t cluster;
    diskfs_inode_t *buf;
    diskfs_inode_t *disk_inode;
    int32_t i;

    /* get disk superblock: */
    disksb = (diskfs_sb_t *) inode->sb->disksb;

    /* calculate the location of the cluster containing the desired inode. */
    inodes_per_cluster = disksb->block_size / sizeof(diskfs_inode_t);
    cluster = disksb->inode_start+inode->ino/inodes_per_cluster;

    /* allocate a buffer: */
    buf = kmalloc(disksb->block_size);
    if (!buf)
        return ENOMEM;

    /* read the cluster into memory: */
    diskfs_read_cluster(inode->sb, cluster, (void *) buf);

    /* filter the desired inode: */
    disk_inode = &buf[inode->ino%inodes_per_cluster];

    /* extract info: */
    inode->ref     = disk_inode->ref;
    inode->mode    = disk_inode->mode;
    inode->devid   = disk_inode->devid;
    inode->size    = disk_inode->size;
    inode->blksize = disksb->block_size;
    inode->blocks  = disk_inode->blocks;

    /* copy block pointers: */
    for (i = 0; i < DISKFS_PTRS; i++)
        inode->info.diskfs.ptr[i] = disk_inode->ptr[i];

    /* no need for the buffer: */
    kfree(buf);

    /* done */
    return ESUCCESS;

}

/****************************************************************************/
/*                             update_inode()                               */
/****************************************************************************/

int32_t diskfs_update_inode(inode_t *inode) {

    /* definitions */
    diskfs_sb_t *disksb;
    uint32_t inodes_per_cluster;
    diskfs_ino_t cluster;
    diskfs_inode_t *buf;
    diskfs_inode_t *disk_inode;
    int32_t i;

    /* get disk superblock: */
    disksb = (diskfs_sb_t *) inode->sb->disksb;

    /* calculate the location of the cluster containing the desired inode. */
    inodes_per_cluster = disksb->block_size / sizeof(diskfs_inode_t);
    cluster = disksb->inode_start + inode->ino/inodes_per_cluster;

    /* allocate a buffer: */
    buf = kmalloc(disksb->block_size);
    if (!buf)
        return ENOMEM;

    /* read the cluster into memory: */
    diskfs_read_cluster(inode->sb, cluster, (void *) buf);

    /* filter the desired inode: */
    disk_inode = &buf[inode->ino%inodes_per_cluster];

    /* update some info: */
    disk_inode->ref    = inode->ref;
    disk_inode->mode   = inode->mode;
    disk_inode->devid  = inode->devid;
    disk_inode->size   = inode->size;
    disk_inode->blocks = inode->blocks;

    /* copy block pointers: */
    for (i = 0; i < DISKFS_PTRS; i++)
        disk_inode->ptr[i] = inode->info.diskfs.ptr[i];

    /* do the update! */
    buf[inode->ino%inodes_per_cluster] = *disk_inode;

    /* write the new updates to disk: */
    diskfs_write_cluster(inode->sb, cluster, (void *) buf);

    /* no need for the buffer: */
    kfree(buf);

    /* done: */
    return ESUCCESS;

}

/****************************************************************************/
/*                             imap_alloc()                                 */
/****************************************************************************/

diskfs_ino_t diskfs_imap_alloc(super_block_t *sb) {

    /* use the inode map to allocate a disk inode.
     * this will just allocate. no initialization is performed.
     */
    diskfs_sb_t *disksb;
    uint8_t *buf;
    int32_t found;
    int32_t bits_per_block;
    diskfs_blk_t cluster;
    int32_t bit_offset;
    int32_t byte;
    int32_t bit;
    diskfs_blk_t loadedClus;
    diskfs_ino_t ino;

    /* read disk superblock: */
    disksb = sb->disksb;

    /* is there any free inode? */
    if (disksb->free_inodes == 0)
        return 0;

    /* allocate buffer: */
    buf = kmalloc(disksb->block_size);
    if (!buf)
        return 0;

    /* some variables that we will use: */
    found = 0; /* flag. */
    bits_per_block = disksb->block_size*8;
    loadedClus = 0; /* currently loaded cluster. */
    ino; /* the number of the found inode. */

    /* loop till finding a free inode... */
    while(!found) {

        /* the cluster that contains the matching bit. */
        cluster = disksb->inode_map_start +
                               disksb->next_free_inode / bits_per_block;

        /* Make sure the desired cluster is loaded. */
        if (cluster != loadedClus)
            diskfs_read_cluster(sb, loadedClus=cluster, buf);

        /* which byte & bit in the cluster? */
        bit_offset = disksb->next_free_inode % bits_per_block;
        byte = bit_offset / 8;
        bit =  bit_offset % 8;

        /* Read the bit! */
        if (!(buf[byte] & (1 << bit))) {
            /* free node detected, reserve it! */
            buf[byte] |= 1 << bit;

            /* update the cluster on disk! */
            diskfs_write_cluster(sb, cluster, buf);

            /* yet another inode is reserved: */
            disksb->free_inodes--;

            /* set ino: */
            ino = disksb->next_free_inode;

            /* we are done. */
            found = 1;
        }

        /* update next free inode value.. */
        if (++disksb->next_free_inode == disksb->total_inodes)
            disksb->next_free_inode = 0;

    }

    /* now update super block: */
    diskfs_write_super(sb);

    /* free the buffer: */
    kfree(buf);

    /* done: */
    return ino;

}

/****************************************************************************/
/*                             imap_free()                                  */
/****************************************************************************/

int32_t diskfs_imap_free(super_block_t *sb, diskfs_ino_t ino) {

    /* free the entry index (ino) from the map. */
    diskfs_sb_t *disksb;
    uint8_t *buf;
    int32_t bits_per_block;
    diskfs_blk_t cluster;
    int32_t bit_offset;
    int32_t byte;
    int32_t bit;

    /* super block: */
    disksb = sb->disksb;

    /* allocate buffer: */
    buf = kmalloc(disksb->block_size);
    if (!buf)
        return ENOMEM;

    /* some variables that we will use: */
    bits_per_block = disksb->block_size*8;
    cluster = disksb->inode_map_start + ino/bits_per_block;
    bit_offset = ino % bits_per_block;
    byte = bit_offset / 8;
    bit  = bit_offset % 8;

    /* load cluster: */
    diskfs_read_cluster(sb, cluster, buf);

    /* free the desired bit: */
    buf[byte] &= ~(1 << bit);

    /* update the cluster on disk! */
    diskfs_write_cluster(sb, cluster, buf);

    /* yet another inode is reserved: */
    disksb->free_inodes++;

    /* now update super block: */
    diskfs_write_super(sb);

    /* free the buffer */
    kfree(buf);

    /* done: */
    return ESUCCESS;

}

/****************************************************************************/
/*                             bmap_alloc()                                 */
/****************************************************************************/

diskfs_blk_t diskfs_bmap_alloc(super_block_t *sb) {

    /* definitions */
    diskfs_sb_t *disksb;
    uint8_t *buf;
    int32_t found;
    int32_t bits_per_block;
    diskfs_blk_t loadedClus;
    diskfs_blk_t blk;
    int32_t bit_offset;
    int32_t byte;
    int32_t bit;
    int32_t i;

    /* get disk superblock: */
    disksb = sb->disksb;

    /* is there any free data cluster? */
    if (disksb->free_data_blocks == 0)
        return 0;

    /* allocate buffer: */
    buf = kmalloc(disksb->block_size);
    if (!buf)
        return 0;

    /* some variables that we will use: */
    found = 0; /* flag. */
    bits_per_block = disksb->block_size*8;
    loadedClus = 0; /* currently loaded cluster. */

    /* loop till finding a free block... */
    while(!found) {

        /* the cluster that contains the matching bit. */
        diskfs_blk_t cluster = disksb->data_map_start +
                               disksb->next_free_block / bits_per_block;

        /* Make sure the desired cluster is loaded. */
        if (cluster != loadedClus)
            diskfs_read_cluster(sb, loadedClus=cluster, buf);

        /* which byte & bit in the cluster? */
        bit_offset = disksb->next_free_block % bits_per_block;
        byte = bit_offset / 8;
        bit  = bit_offset % 8;

        /* Read the bit! */
        if (!(buf[byte] & (1 << bit))) {
            /* free block detected, reserve it! */
            buf[byte] |= 1 << bit;

            /* update the cluster on disk! */
            diskfs_write_cluster(sb, cluster, buf);

            /* yet another block is reserved: */
            disksb->free_data_blocks--;

            /* set blk: */
            blk = disksb->next_free_block + disksb->data_start;

            /* we are done. */
            found = 1;
        }

        /* update next free free block value.. */
        if (++disksb->next_free_block == disksb->data_blocks)
            disksb->next_free_block = 0;

    }

    /* now update super block: */
    diskfs_write_super(sb);

    /* zeroise buffer */
    for (i = 0; i < sb->blksize; i++)
        buf[i] = 0;

    /* write the zeros to the allocated cluster */
    diskfs_write_cluster(sb, blk, buf);

    /* free the buffer: */
    kfree(buf);

    /* return: */
    return blk;
}

/****************************************************************************/
/*                             bmap_free()                                  */
/****************************************************************************/

int32_t diskfs_bmap_free(super_block_t *sb, diskfs_blk_t blk) {

    /* free the entry of (blk) on the data block map. */
    diskfs_sb_t *disksb;
    uint8_t *buf;
    int32_t bits_per_block;
    diskfs_blk_t cluster;
    int32_t bit_offset;
    int32_t byte;
    int32_t bit;

    /* super block: */
    disksb = sb->disksb;

    /* allocate buffer: */
    buf = kmalloc(disksb->block_size);
    if (!buf)
        return ENOMEM;

    /* some variables that we will use: */
    bits_per_block = disksb->block_size*8;
    cluster = disksb->data_map_start+(blk-disksb->data_start)/bits_per_block;
    bit_offset = (blk-disksb->data_start) % bits_per_block;
    byte = bit_offset / 8;
    bit  = bit_offset % 8;

    /* load cluster: */
    diskfs_read_cluster(sb, cluster, buf);

    /* free the desired bit: */
    buf[byte] &= ~(1 << bit);

    /* update the cluster on disk! */
    diskfs_write_cluster(sb, cluster, buf);

    /* yet another inode is reserved: */
    disksb->free_inodes++;

    /* now update super block: */
    diskfs_write_super(sb);

    /* free the buffer */
    kfree(buf);

    /* done: */
    return ESUCCESS;

}

/****************************************************************************/
/*                              read_fileblk()                              */
/****************************************************************************/

int32_t diskfs_read_fileblk(inode_t *inode,
                            diskfs_blk_t blk_off,
                            void *buf) {

    /* convert blk_off (which is relative to file)
     * to a block number relative to the beginning
     * of the filesystem, then read that block!
     */

    /* calculate level & loop parameters: */
    diskfs_lvl_t lvl = diskfs_blk_to_lvl(inode->sb->disksb, blk_off);
    diskfs_blk_t  blk;
    diskfs_blk_t *ptr;
    int32_t i;

    /* loop on all levels: */
    for (i = 0; i <= lvl.level; i++) {

        /* load the table into memory: */
        if (!i) {
            ptr = inode->info.diskfs.ptr;
        } else {
            diskfs_read_cluster(inode->sb, blk, buf);
            ptr = (diskfs_blk_t *) buf;
        }

        /* get pointer for next level (if there isn't): */
        if (!(blk = ptr[lvl.ptr[i]])) {
            break; /* exit the loop. */
        }

    }

    /* zero? */
    if (!blk) {
        for(i = 0; i < inode->sb->blksize; i++)
            ((uint8_t *) buf)[i] = 0;
    } else {
        diskfs_read_cluster(inode->sb, blk, buf);
    }

    /* done: */
    return ESUCCESS;

}

/****************************************************************************/
/*                             write_fileblk()                              */
/****************************************************************************/

int32_t diskfs_write_fileblk(inode_t *inode,
                             diskfs_blk_t blk_off,
                             void *buf) {

    /* allocate a block for the file
     * referenced by "inode". the block
     * is allocated at the offset "blk_off"
     * from the beginning of the file.
     * if there is already an allocated block,
     * don't allocate another one.
     */
    int32_t ptrs_per_block;
    diskfs_lvl_t lvl;
    uint8_t *tmp;
    int32_t init = 0, i, j = 0;
    diskfs_blk_t  blk;
    diskfs_blk_t *ptr;

    /* calculate stuff: */
    ptrs_per_block = inode->sb->blksize/sizeof(diskfs_blk_t);
    lvl = diskfs_blk_to_lvl(inode->sb->disksb, blk_off);

    /* allocate buffer: */
    tmp = kmalloc(inode->sb->blksize);
    if (!tmp)
        return 0;

    for (i = 0; i <= lvl.level; i++) {

        /* load the table into memory: */
        if (!i) {
            ptr = inode->info.diskfs.ptr;
        } else {
            diskfs_read_cluster(inode->sb, blk, tmp);
            ptr = (diskfs_blk_t *) tmp;
        }

        /* initialize the table? */
        if (init) {
            while(j < ptrs_per_block)
                ptr[j++] = 0;
            init = 0;
        }

        /* allocate a pointer for next level (if there isn't): */
        if (ptr[lvl.ptr[i]]) {

            blk = ptr[lvl.ptr[i]]; /* next level; */

        } else {

            /* allocate and update: */
            ptr[lvl.ptr[i]] = diskfs_bmap_alloc(inode->sb);

            /* the new allocated page need to be initialized: */
            init = 1;

            /* write updates to disk */
            if (!i)
                diskfs_t.update_inode(inode);
            else
                diskfs_write_cluster(inode->sb, blk, tmp);

            /* set blk to next level; */
            blk = ptr[lvl.ptr[i]];

            /* NULL? */
            if (!blk)
                return 0;

        }
    }

    /* no need for tmp: */
    kfree(tmp);

    /* write block: */
    diskfs_write_cluster(inode->sb, blk, buf);

    /* done */
    return ESUCCESS;

}

/****************************************************************************/
/*                              free_fileblk()                              */
/****************************************************************************/

void diskfs_free_fileblk(inode_t *inode, diskfs_blk_t blk_off) {

    /* variables that are used */
    int32_t ptrs_per_block;
    diskfs_lvl_t lvl;
    uint8_t *tmp;
    int32_t init = 0, i, j;
    diskfs_blk_t  blk[5] = {0};
    diskfs_blk_t *ptr;

    /* calculate stuff: */
    ptrs_per_block = inode->sb->blksize/sizeof(diskfs_blk_t);
    lvl = diskfs_blk_to_lvl(inode->sb->disksb, blk_off);

    /* allocate buffer: */
    tmp = kmalloc(inode->sb->blksize);
    if (!tmp)
        return;

    /* loop on all levels: */
    for (i = 0; i <= lvl.level; i++) {

        /* load the table into memory: */
        if (!i) {
            ptr = inode->info.diskfs.ptr;
        } else {
            diskfs_read_cluster(inode->sb, blk[i-1], tmp);
            ptr = (diskfs_blk_t *) tmp;
        }

        /* get pointer for next level (if there isn't): */
        if (!(blk[i+1] = ptr[lvl.ptr[i]])) {
            break;
        }
    }


    for (i = lvl.level+1; i >= 0; i--) {
        /* start removing blocks & tables level by level. */
        if (i == lvl.level+1) {
            if (!blk[i])
                continue;
            diskfs_bmap_free(inode->sb, blk[i]);
        } else if (i > 0) {
            /* a table. */
            if (!blk[i])
                continue;
            diskfs_read_cluster(inode->sb, blk[i], tmp);
            ptr = (diskfs_blk_t *) tmp;
            ptr[lvl.ptr[i]] = 0;
            diskfs_write_cluster(inode->sb, blk[i], tmp);
            /* all zeros? */
            for (j = 0; j < ptrs_per_block && (!ptr[j]); j++);
            if (j != ptrs_per_block) {
                /* omg i can't delete the table.. */
                break;
            }
            diskfs_bmap_free(inode->sb, blk[i]);
        } else {
            /* we are at the inode level. */
            ptr = inode->info.diskfs.ptr;
            ptr[lvl.ptr[i]] = 0;
            diskfs_update_inode(inode);
        }
    }

    /* done */
    return;

}

/***************************************************************************/
/*                               put_inode()                               */
/***************************************************************************/

int32_t diskfs_put_inode(inode_t *inode) {

    /* local vars */
    diskfs_blk_t blocks;
    int32_t dirents_per_block;
    int32_t i = 0; /* counter for blocks. */
    int32_t j = 0; /* counter for dirents per block. */
    diskfs_blk_t b;
    diskfs_dirent_t *dirents;

    /* inode is still referenced? */
    if (inode->icount || inode->ref)
        return ESUCCESS;

    /* remove the inode 3:) */
    blocks = 0;

    /* get count of blocks: */
    if ((inode->mode & FT_MASK) == FT_DIR) {
        /* calculate count of directory blocks: */
        dirents = kmalloc(inode->blksize);
        if (!dirents) {
            return ENOMEM;
        }

        /* initialize counters: */
        dirents_per_block = inode->blksize/sizeof(diskfs_dirent_t);
        i = 0; /* counter for blocks. */
        j = 0; /* counter for dirents per block. */

        /* loop on dirents. */
        while(1) {
            /* beginning of a new block? */
            if (j == 0)
                diskfs_read_fileblk(inode, i++, (uint8_t *) dirents);

            /* done? */
            if (dirents[j].ino == 0)
                break;

            /* next dirent: */
            if (++j == dirents_per_block)
                j = 0;
        }

        /* now i contains the count of blocks to free: */
        blocks = i;

        /* deallocate the buffer: */
        kfree(dirents);
    } else {
        /* just a file: */
        blocks = inode->blocks;
    }

    /* remove all blocks: */
    for (b = 0; b < blocks; b++)
        diskfs_free_fileblk(inode, b);

    /* debug inode pointers: */
    /*for (i = 0; i < 15; i++) */
    /*    printk("%d: %x\n", i, inode->info.diskfs.ptr[i]); */
    /*printk("=======================================\n"); */

    /* now free the inode itself. */
    diskfs_imap_free(inode->sb, inode->ino);

    /* done */
    return ESUCCESS;

}

/***************************************************************************/
/*                                lookup()                                 */
/***************************************************************************/

int32_t diskfs_lookup(inode_t *dir, char *name, inode_t **ret) {

    /* local vars */
    diskfs_dirent_t *dirents;
    int32_t dirents_per_block;
    int32_t i;
    int32_t j;

    /* allocate buffer: */
    dirents = kmalloc(dir->blksize);
    if (!dirents)
        return ENOMEM;

    /* initialize counters: */
    dirents_per_block = dir->blksize/sizeof(diskfs_dirent_t);
    i = 0; /* counter for blocks. */
    j = 0; /* counter for dirents per block. */

    /* loop on dirents. */
    while(1) {
        /* beginning of a new block? */
        if (j == 0)
            diskfs_read_fileblk(dir, i++, dirents);

        /* done? */
        if (dirents[j].ino == 0) {
            kfree(dirents);
            return ENOENT;
        }

        /* name matching? */
        if (dirents[j].ino > 1 && !strcmp(dirents[j].name, name)) {
            if (ret) {
                *ret = (inode_t *) iget(dir->sb, dirents[j].ino);
                if (!(*ret)) {
                    kfree(dirents);
                    return ENOMEM;
                }
            }
            kfree(dirents);
            return ESUCCESS;
        }

        /* next dirent: */
        if (++j == dirents_per_block)
            j = 0;
    }

}

/***************************************************************************/
/*                                mknod()                                  */
/***************************************************************************/

int32_t diskfs_mknod(inode_t *dir, char *name, int32_t mode, int32_t devid) {

    int32_t i, j;
    int32_t err;
    diskfs_dirent_t *buf;
    diskfs_ino_t ino;
    inode_t *inode;
    int32_t dirents_per_block;

    /* dir must be directory: */
    if ((dir->mode & FT_MASK) != FT_DIR)
        return ENOTDIR;

    /* dir is already deleted? */
    if (!(dir->ref))
        return ENOENT;

    /* file exists? */
    err = diskfs_lookup(dir, name, NULL);
    if (!err)
        return EEXIST;
    else if (err != ENOENT)
        return err;

    /* allocate buffer: */
    buf = kmalloc(dir->blksize);
    if (!buf)
        return ENOMEM;

    /* allocate a disk inode: */
    ino = diskfs_imap_alloc(dir->sb);
    if (!ino)
        return ENOSPC;

    /* read the inode: */
    inode = (inode_t *) iget(dir->sb, ino);
    if (!inode)
        return ENOMEM;

    /* initialize the inode: */
    inode->ref  = 1;
    inode->mode = mode;
    inode->size = 0;
    inode->blocks = 0;
    inode->devid = devid;
    for (i = 0; i < DISKFS_PTRS; i++)
        inode->info.diskfs.ptr[i] = 0;

    /* update the inode: */
    diskfs_update_inode(inode);

    /* insert a new entry in "dir": */
    dirents_per_block = inode->blksize/sizeof(diskfs_dirent_t);
    i = 0; /* counter for blocks. */
    j = 0; /* counter for dirents per block. */

    /* loop on dirents. */
    while(1) {
        /* beginning of a new block? */
        if (j == 0)
            diskfs_read_fileblk(dir, i++, buf);

        /* a free entry? */
        if (buf[j].ino == 0 || buf[j].ino == 1) {
            buf[j].ino = ino;
            strcpy(buf[j].name, name);
            diskfs_write_fileblk(dir, i-1, buf);
            break;
        }

        /* next dirent: */
        if (++j == dirents_per_block)
            j = 0;
    }

    /* put the inode: */
    iput(inode);

    /* free the buffer: */
    kfree(buf);

    /* done: */
    return ESUCCESS;

}

/***************************************************************************/
/*                                link()                                   */
/***************************************************************************/

int32_t diskfs_link(inode_t *inode, inode_t *dir, char *name) {

    /* local vars */
    int32_t err;
    diskfs_dirent_t *buf;
    int32_t dirents_per_block;
    int32_t i;
    int32_t j;

    /* dir must be directory: */
    if ((dir->mode & FT_MASK) != FT_DIR)
        return ENOTDIR;

    /* dir is already deleted? */
    if (!(dir->ref))
        return ENOENT;

    /* file exists? */
    err = diskfs_lookup(dir, name, NULL);
    if (!err)
        return EEXIST;
    else if (err != ENOENT)
        return err;

    /* inode should not be directory: */
    if ((inode->mode & FT_MASK) == FT_DIR)
        return EISDIR;

    /* allocate buffer: */
    buf = kmalloc(dir->blksize);
    if (!buf)
        return ENOMEM;

    /* increase references count: */
    inode->ref++;
    diskfs_update_inode(inode);

    /* insert a new entry in "dir": */
    dirents_per_block = inode->blksize/sizeof(diskfs_dirent_t);
    i = 0; /* counter for blocks. */
    j = 0; /* counter for dirents per block. */

    /* loop on dirents. */
    while(1) {
        /* beginning of a new block? */
        if (j == 0)
            diskfs_read_fileblk(dir, i++, buf);

        /* a free entry? */
        if (buf[j].ino == 0 || buf[j].ino == 1) {
            buf[j].ino = inode->ino;
            strcpy(buf[j].name, name);
            diskfs_write_fileblk(dir, i-1, buf);
            break;
        }

        /* next dirent: */
        if (++j == dirents_per_block)
            j = 0;
    }

    /* free the buffer: */
    kfree(buf);

    /* done: */
    return ESUCCESS;
}

/***************************************************************************/
/*                               unlink()                                  */
/***************************************************************************/

int32_t diskfs_unlink(inode_t *dir, char *name) {

    /* local vars */
    diskfs_dirent_t *dirents;
    int32_t dirents_per_block;
    int32_t i;
    int32_t j;

    /* dir must be directory: */
    if ((dir->mode & FT_MASK) != FT_DIR)
        return ENOTDIR;

    /* dir is already deleted? */
    if (!(dir->ref))
        return ENOENT;

    /* allocate buffer: */
    dirents = kmalloc(dir->blksize);
    if (!dirents)
        return ENOMEM;

    /* initialize counters: */
    dirents_per_block = dir->blksize/sizeof(diskfs_dirent_t);
    i = 0; /* counter for blocks. */
    j = 0; /* counter for dirents per block. */

    /* loop on dirents. */
    while(1) {
        /* beginning of a new block? */
        if (j == 0)
            diskfs_read_fileblk(dir, i++, (char *) dirents);

        /* done? */
        if (dirents[j].ino == 0) {
            kfree(dirents);
            return ENOENT;
        }

        /* name matching? */
        if (dirents[j].ino > 1 && !strcmp(dirents[j].name, name)) {

            /* get the inode: */
            inode_t *inode = (inode_t *) iget(dir->sb,
                                  dirents[j].ino);

            /* directory? */
            if ((inode->mode & FT_MASK) == FT_DIR) {
                iput(inode);
                kfree(dirents);
                return EISDIR;
            }

            /* decrease references: */
            inode->ref--;
            diskfs_update_inode(inode);

            /* remove entry: */
            dirents[j].ino = 1;
            diskfs_write_fileblk(dir, i-1, dirents);
            kfree(dirents);

            /* put the inode: */
            iput(inode);

            /* done: */
            return ESUCCESS;

        }

        /* next dirent: */
        if (++j == dirents_per_block)
            j = 0;
    }

}

/***************************************************************************/
/*                                mkdir()                                  */
/***************************************************************************/

int32_t diskfs_mkdir(inode_t *dir, char *name, int32_t mode) {

    /* local vars */
    int32_t i, j;
    int32_t err;
    diskfs_dirent_t *buf;
    diskfs_ino_t ino;
    inode_t *inode;
    int32_t dirents_per_block;

    /* dir must be directory: */
    if ((dir->mode & FT_MASK) != FT_DIR)
        return ENOTDIR;

    /* dir is already deleted? */
    if (!(dir->ref))
        return ENOENT;

    /* file exists? */
    err = diskfs_lookup(dir, name, NULL);
    if (!err)
        return EEXIST;
    else if (err != ENOENT)
        return err;

    /* allocate buffer: */
    buf = kmalloc(dir->blksize);
    if (!buf)
        return ENOMEM;

    /* allocate a disk inode: */
    ino = diskfs_imap_alloc(dir->sb);
    if (!ino)
        return ENOSPC;

    /* read the inode: */
    inode = (inode_t *) iget(dir->sb, ino);
    if (!inode)
        return ENOMEM;

    /* initialize the inode: */
    inode->ref  = 1;
    inode->mode = mode;
    inode->size = 0;
    inode->blocks = 0;
    inode->devid = 0;
    for (i = 0; i < DISKFS_PTRS; i++)
        inode->info.diskfs.ptr[i] = 0;

    /* update the inode: */
    diskfs_update_inode(inode);

    /* insert "dot" and "dotdot": */
    for (i = 0; i < inode->blksize; i++)
        ((char *) buf)[i] = 0;
    buf[0].ino = inode->ino;
    buf[0].name[0] = '.';
    buf[0].name[1] = 0;
    buf[1].ino = dir->ino;
    buf[1].name[0] = '.';
    buf[1].name[1] = '.';
    buf[1].name[2] = 0;
    diskfs_write_fileblk(inode, 0, buf);

    /* insert a new entry in "dir": */
    dirents_per_block = inode->blksize/sizeof(diskfs_dirent_t);
    i = 0; /* counter for blocks. */
    j = 0; /* counter for dirents per block. */

    /* loop on dirents. */
    while(1) {
        /* beginning of a new block? */
        if (j == 0)
            diskfs_read_fileblk(dir, i++, buf);

        /* a free entry? */
        if (buf[j].ino == 0 || buf[j].ino == 1) {
            buf[j].ino = ino;
            strcpy(buf[j].name, name);
            diskfs_write_fileblk(dir, i-1, buf);
            break;
        }

        /* next dirent: */
        if (++j == dirents_per_block)
            j = 0;
    }

    /* put the inode: */
    iput(inode);

    /* free the buffer: */
    kfree(buf);

    /* done: */
    return ESUCCESS;

}

/***************************************************************************/
/*                                rmdir()                                  */
/***************************************************************************/

int32_t diskfs_rmdir(inode_t *dir, char *name) {

    int32_t i, j;
    int32_t err;
    inode_t *inode;
    diskfs_dirent_t *buf;
    int32_t dirents_per_block;
    pos_t entcount;

    /* dir must be directory: */
    if ((dir->mode & FT_MASK) != FT_DIR)
        return ENOTDIR;

    /* dir is already deleted? */
    if (!(dir->ref))
        return ENOENT;

    /* file doesn't exist? */
    err = diskfs_lookup(dir, name, &inode);
    if (err)
        return err;

    /* file must be directory: */
    if ((inode->mode & FT_MASK) != FT_DIR) {
        iput(inode);
        return ENOTDIR;
    }

    /* allocate buffer: */
    buf = kmalloc(dir->blksize);
    if (!buf) {
        iput(inode);
        return ENOMEM;
    }

    /* get count of entries in the target: */
    dirents_per_block = dir->blksize/sizeof(diskfs_dirent_t);
    i = 0; /* counter for blocks. */
    j = 0; /* counter for dirents per block. */
    entcount = 0;

    /* loop on dirents. */
    while(1) {
        /* beginning of a new block? */
        if (j == 0)
            diskfs_read_fileblk(inode, i++, buf);

        /* done? */
        if (buf[j].ino == 0)
            break;

        /* valid entry */
        if (buf[j].ino > 1)
            entcount++;

        /* next dirent: */
        if (++j == dirents_per_block)
            j = 0;
    }

    /* directory is not empty? */
    if (entcount > 2) {
        iput(inode);
        kfree(buf);
        return ENOTEMPTY;
    }

    /* remove entry from parent directory: */
    i = 0; /* counter for blocks. */
    j = 0; /* counter for dirents per block. */

    /* loop on dirents. */
    while(1) {
        /* beginning of a new block? */
        if (j == 0)
            diskfs_read_fileblk(dir, i++, buf);

        /* done? */
        if (buf[j].ino == 0) {
            iput(inode);
            kfree(buf);
            return ENOENT;
        }

        /* name matching? */
        if (buf[j].ino > 1 && !strcmp(buf[j].name, name)) {

            /* remove entry: */
            buf[j].ino = 1;
            diskfs_write_fileblk(dir, i-1, buf);
            break;

        }

        /* next dirent: */
        if (++j == dirents_per_block)
            j = 0;
    }

    /* decrease references: */
    inode->ref--;
    diskfs_update_inode(inode);

    /* put the inode: */
    iput(inode);

    /* free the buffer: */
    kfree(buf);

    /* done: */
    return ESUCCESS;

}

/***************************************************************************/
/*                               truncate()                                */
/***************************************************************************/

int32_t diskfs_truncate(inode_t *inode, pos_t length) {

    /* init some vars */
    diskfs_blk_t i;
    pos_t oldsize;
    pos_t newsize;
    diskfs_blk_t last_blk;
    diskfs_blk_t start_blk;

    /* inode should be a regular file */
    if ((inode->mode & FT_MASK) != FT_REGULAR)
        return EINVAL;

    /* initialize data */
    oldsize   = inode->size;
    newsize   = length;
    last_blk  = oldsize/inode->blksize;
    start_blk = newsize/inode->blksize + (newsize%inode->blksize ? 1 : 0);

    /* delete truncated blocks: */
    for (i = start_blk; i <= last_blk; i++)
        diskfs_free_fileblk(inode, i);

    /* update file size... */
    inode->size = newsize;
    inode->blocks = (inode->size/inode->blksize) +
                    ((inode->size%inode->blksize) ? 1:0);
    diskfs_update_inode(inode);

    /* done: */
    return ESUCCESS;

}

/***************************************************************************/
/*                                 open()                                  */
/***************************************************************************/

int32_t diskfs_open(file_t *file) {

    /* mark buffer as empty: */
    file->info.diskfs.buf_empty = 1;

    /* done */
    return ESUCCESS;

}

/***************************************************************************/
/*                                release()                                */
/***************************************************************************/

int32_t diskfs_release(file_t *file) {

    /* free the buffer: */
    if (!(file->info.diskfs.buf_empty))
        kfree(file->info.diskfs.buffer);

    /* done */
    return ESUCCESS;

}

/***************************************************************************/
/*                                 read()                                  */
/***************************************************************************/

int32_t diskfs_read(file_t *file, void *buf, int32_t size) {

    pos_t off = file->pos;
    int32_t rem = size; /* remaining */
    int32_t blksize = file->inode->blksize;
    int32_t i, j;
    pos_t tsize; /* size of the transfer */
    int32_t count;

    if (size <= 0)
        return EINVAL; /* invalid */

    if (off >= file->inode->size)
        return 0; /* EOF. */

    if (off + rem > file->inode->size)
        rem = file->inode->size - off;

    /* read block by block.. */
    while(rem) {

        /* try to not skip current block. */
        if ((tsize = blksize-off%blksize) > rem)
            tsize = rem;

        /* do the read! */
        if (tsize == blksize) {
            /* read the whole block! */
            diskfs_read_fileblk(file->inode, off/blksize, buf);
        } else {
            /* we need to make use of the temp buffer... */
            if (file->info.diskfs.buf_empty) {
                /* allocate buffer: */
                file->info.diskfs.buffer = kmalloc(blksize);
                if (!(file->info.diskfs.buffer))
                    return ENOMEM; /* no memory. */

                /* load current cluster into the buffer. */
                diskfs_read_fileblk(file->inode,
                                    off/blksize,
                                    file->info.diskfs.buffer);

                /* update current buffered cluster value: */
                file->info.diskfs.buf_blk = off/blksize;

                /* buffer is not empty now: */
                file->info.diskfs.buf_empty = 0;
            } else if (file->info.diskfs.buf_blk != off/blksize) {
                /* shame on the buffer! another cluster is loaded! */

                /* load current cluster into the buffer. */
                diskfs_read_fileblk(file->inode,
                                    off/blksize,
                                    file->info.diskfs.buffer);

                /* update current buffered cluster value: */
                file->info.diskfs.buf_blk = off/blksize;
            }

            /* now it is guaranteed that the buffer
             * contains the desired cluster.
             * copy data byte by byte.
             */
            for (i=off%blksize,j=0; i<off%blksize+tsize; i++,j++)
                ((uint8_t *)buf)[j] = file->info.diskfs.buffer[i];

        }

        /* update remaining: */
        rem -= tsize;
        off += tsize;
        buf = ((uint8_t *) buf) + tsize;

    }

    /* how much has been transfered? */
    count = off - file->pos;
    file->pos = off;
    return ESUCCESS;

}

/***************************************************************************/
/*                                 write()                                 */
/***************************************************************************/

int32_t diskfs_write(file_t *file, void *buf, int32_t size) {

    pos_t off = file->pos;
    int32_t rem = size; /* remaining */
    int32_t blksize = file->inode->blksize;
    int32_t i, j;
    pos_t tsize;
    int32_t count;

    if (size <= 0)
        return EINVAL; /* invalid */

    if (off + rem > file->inode->size) {
        /* update file size... */
        file->inode->size = off+rem;
        file->inode->blocks = (file->inode->size/blksize) +
                              ((file->inode->size%blksize) ? 1:0);
        diskfs_update_inode(file->inode);
    }

    /* write block by block.. */
    while(rem) {

        /* try to not skip current block. */
        if ((tsize = blksize-off%blksize) > rem)
            tsize = rem;

        /* do the write! */
        if (tsize == blksize) {
            /* write the whole block! */
            diskfs_write_fileblk(file->inode, off/blksize, buf);
        } else {
            /* we need to make use of the temp buffer... */
            if (file->info.diskfs.buf_empty) {
                /* allocate buffer: */
                file->info.diskfs.buffer = kmalloc(blksize);
                if (!(file->info.diskfs.buffer))
                    return ENOMEM; /* no memory. */

                /* load current cluster into the buffer. */
                diskfs_read_fileblk(file->inode,
                                    off/blksize,
                                    file->info.diskfs.buffer);

                /* update current buffered cluster value: */
                file->info.diskfs.buf_blk = off/blksize;

                /* buffer is not empty now: */
                file->info.diskfs.buf_empty = 0;
            } else if (file->info.diskfs.buf_blk != off/blksize) {
                /* shame on the buffer! another cluster is loaded! */

                /* load current cluster into the buffer. */
                diskfs_read_fileblk(file->inode,
                                    off/blksize,
                                    file->info.diskfs.buffer);

                /* update current buffered cluster value: */
                file->info.diskfs.buf_blk = off/blksize;
            }

            /* now it is guaranteed that the temp buffer
             * contains the desired cluster.
             * copy data byte by byte to the temp buffer.
             */
            for (i=off%blksize,j=0; i<off%blksize+tsize; i++,j++)
                file->info.diskfs.buffer[i] = ((uint8_t *) buf)[j];

            /* now write the block: */
            diskfs_write_fileblk(file->inode,
                                 off/blksize,
                                 file->info.diskfs.buffer);

        }

        /* update remaining: */
        rem -= tsize;
        off += tsize;
        buf = ((uint8_t *) buf) + tsize;

    }

    /* how much has been transfered? */
    count = off - file->pos;
    file->pos = off;
    return 0;

}

/***************************************************************************/
/*                                 seek()                                  */
/***************************************************************************/

int32_t diskfs_seek(file_t *file, pos_t newpos) {

    /* just set file->pos, no need to update buffers now. */
    file->pos = newpos;
    return 0;

}

/***************************************************************************/
/*                                 readdir()                               */
/***************************************************************************/

int32_t diskfs_readdir(file_t *file, dirent_t *dirent) {

    /* loop on directory entries until find a proper entry, or null. */
    diskfs_blk_t blk;
    int32_t off;
    diskfs_dirent_t *ent;

    while(1) {
        /* calculate current block: */
        blk = file->pos / file->inode->blksize;

        /* calculate current offset inside the block: */
        off = file->pos % file->inode->blksize;

        /* buffer is ready? */
        if (file->info.diskfs.buf_empty) {
            /* allocate buffer: */
            file->info.diskfs.buffer = kmalloc(file->inode->blksize);
            if (!(file->info.diskfs.buffer))
                return 0; /* no memory. */

            /* load current cluster into the buffer. */
            diskfs_read_fileblk(file->inode,
                                blk,
                                file->info.diskfs.buffer);

            /* update current buffered cluster value: */
            file->info.diskfs.buf_blk = blk;

            /* buffer is not empty now: */
            file->info.diskfs.buf_empty = 0;
        } else if (file->info.diskfs.buf_blk != blk) {
            /* shame on the buffer! another cluster is loaded! */

            /* load current cluster into the buffer. */
            diskfs_read_fileblk(file->inode,
                                blk,
                                file->info.diskfs.buffer);

            /* update current buffered cluster value: */
            file->info.diskfs.buf_blk = blk;
        }

        /* get current entry: */
        ent = (diskfs_dirent_t *) &file->info.diskfs.buffer[off];

        /* done? */
        if (ent->ino == 0)
            return 0; /* no more entries. */

        /* a proper entry? */
        if (ent->ino > 1) {
            dirent->ino = ent->ino;
            strcpy(dirent->name, ent->name);
            file->pos += sizeof(diskfs_dirent_t);
            return 1;
        }

        /* just update pos and continue the loop: */
        file->pos += sizeof(diskfs_dirent_t);

    }

}

/***************************************************************************/
/*                                 ioctl()                                 */
/***************************************************************************/

int32_t diskfs_ioctl(file_t *file, int32_t cmd, void *arg) {

    /* currently diskfs doesn't make use of this... */
    return EBUSY;

}

/***************************************************************************/
/*                               diskfsls()                                */
/***************************************************************************/

void diskfsls(char *path) {

    /* i use this to debug my driver... */

    file_t *file;
    int32_t err;
    dirent_t dir;

    printk("$ ls %s\n", path);

    err = file_open(path, 0, &file);

    if (err) {
        printk("error happened during opening.\n");
        return;
    }

    if (strcmp(file->inode->sb->fsdriver->alias, "diskfs")) {
        printk("not diskfs!\n");
        return;
    }

    if ((file->inode->mode & FT_MASK) != FT_DIR) {
        printk("not directory!\n");
        return;
    }

    while(1) {
        if (!file_readdir(file, &dir))
            break; /*done. */
        printk("%d: %s\n", dir.ino, dir.name);
    }

    file_close(file);

}

/***************************************************************************/
/*                               fsdriver_t                                */
/***************************************************************************/

fsd_t diskfs_t = {

    /* alias:        */ "diskfs",
    /* flags:        */ FSD_REQDEV,

    /* read_super:   */ diskfs_read_super,
    /* write_super:  */ diskfs_write_super,
    /* put_super:    */ diskfs_put_super,
    /* read_inode:   */ diskfs_read_inode,
    /* update_inode: */ diskfs_update_inode,
    /* put_inode:    */ diskfs_put_inode,

    /* lookup:       */ diskfs_lookup,
    /* mknod:        */ diskfs_mknod,
    /* link:         */ diskfs_link,
    /* unlink:       */ diskfs_unlink,
    /* mkdir:        */ diskfs_mkdir,
    /* rmdir:        */ diskfs_rmdir,
    /* truncate:     */ diskfs_truncate,

    /* open:         */ diskfs_open,
    /* release:      */ diskfs_release,
    /* read:         */ diskfs_read,
    /* write:        */ diskfs_write,
    /* seek:         */ diskfs_seek,
    /* readdir:      */ diskfs_readdir,
    /* ioctl:        */ diskfs_ioctl

};
