/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> System Filesystem Driver.                        | |
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
#include <lib/string.h>
#include <sys/mm.h>
#include <sys/fs.h>
#include <fs/tmpfs.h>

super_block_t *sysfs_sb = NULL;

typedef struct sysfile {
    struct sysfile *next;
    char *name;
    ino_t ino;
    char *(*read)(uint32_t *size);
} sysfile_t;

sysfile_t *sysfiles = NULL;

/***************************************************************************/
/*                              read_super                                 */
/***************************************************************************/

super_block_t *sysfs_read_super(device_t *dev) {
    if (sysfs_sb)
        return sysfs_sb;
    else {
        int i;
        inode_t *root;
        sysfile_t *cur = sysfiles;
        /* mount tmpfs */
        sysfs_sb = (super_block_t *) tmpfs_read_super(dev);
        sysfs_sb->fsdriver = &sysfs_t;
        /* create system files */
        root = (inode_t *) iget(sysfs_sb, sysfs_sb->root_ino);
        while (cur) {
            inode_t *child;
            tmpfs_mknod(root, cur->name, FT_REGULAR, 0);
            tmpfs_lookup(root, cur->name, &child);
            cur->ino = child->ino;
            iput(child);
            cur = cur->next;
        }
        iput(root);
        /* done */
        return sysfs_sb;
    }
}

/***************************************************************************/
/*                              write_super                                */
/***************************************************************************/

int32_t sysfs_write_super(super_block_t *sb) {
    return tmpfs_write_super(sb);
}

/***************************************************************************/
/*                               put_super                                 */
/***************************************************************************/

int32_t sysfs_put_super(super_block_t *sb) {
    return ESUCCESS;
}

/***************************************************************************/
/*                              read_inode()                               */
/***************************************************************************/

int32_t sysfs_read_inode(inode_t *inode) {
    return tmpfs_read_inode(inode);
}

/***************************************************************************/
/*                             update_inode()                              */
/***************************************************************************/

int32_t sysfs_update_inode(inode_t *inode) {
    return tmpfs_update_inode(inode);
}

/***************************************************************************/
/*                              put_inode()                                */
/***************************************************************************/

int32_t sysfs_put_inode(inode_t *inode) {
    return tmpfs_put_inode(inode);
}

/***************************************************************************/
/*                                lookup()                                 */
/***************************************************************************/

int32_t sysfs_lookup(inode_t *dir, char *name, inode_t **ret) {
    return tmpfs_lookup(dir, name, ret);
}

/***************************************************************************/
/*                                 mknod()                                 */
/***************************************************************************/

int32_t sysfs_mknod(inode_t *dir, char *name, int32_t mode, int32_t devid) {
    return EIO;
}

/***************************************************************************/
/*                                link()                                   */
/***************************************************************************/

int32_t sysfs_link(inode_t *inode, inode_t *dir, char *name) {
    return EIO;
}

/***************************************************************************/
/*                               unlink()                                  */
/***************************************************************************/

int32_t sysfs_unlink(inode_t *dir, char *name) {
    return EIO;
}

/***************************************************************************/
/*                                mkdir()                                  */
/***************************************************************************/

int32_t sysfs_mkdir(inode_t *dir, char *name, int32_t mode) {
    return EIO;
}

/***************************************************************************/
/*                                rmdir()                                  */
/***************************************************************************/

int32_t sysfs_rmdir(inode_t *dir, char *name) {
    return EIO;
}

/***************************************************************************/
/*                               truncate()                                */
/***************************************************************************/

int32_t sysfs_truncate(inode_t *inode, pos_t length) {
    return EIO;
}

/***************************************************************************/
/*                                 open()                                 */
/***************************************************************************/

int32_t sysfs_open(file_t *file) {
    if ((file->inode->mode & FT_MASK) == FT_REGULAR) {
        sysfile_t *cur = sysfiles;
        char *(*read)(uint32_t *size);
        while (cur && cur->ino != file->inode->ino) {
            cur = cur->next;
        }
        if (!cur)
            return EIO;
        read = cur->read;
        file->info.tmpfs.dummy_p = read(&file->info.tmpfs.dummy_i);
        if (!file->info.tmpfs.dummy_p)
            return ENOMEM;
        else
            return ESUCCESS;
    } else {
        return tmpfs_open(file);
    }
}

/***************************************************************************/
/*                                release()                                */
/***************************************************************************/

int32_t sysfs_release(file_t *file) {
    if ((file->inode->mode & FT_MASK) == FT_REGULAR) {
        kfree(file->info.tmpfs.dummy_p);
        return ESUCCESS;
    } else {
        return tmpfs_release(file);
    }
}

/***************************************************************************/
/*                                 read()                                  */
/***************************************************************************/

int32_t sysfs_read(file_t *file, void *buf, int32_t size) {
    char *cbuf = buf;
    if ((file->inode->mode & FT_MASK) == FT_REGULAR) {
        while(size) {
            if (file->pos >= file->info.tmpfs.dummy_i) {
                /* EOF */
                return ESUCCESS;
            }
            *cbuf++ = file->info.tmpfs.dummy_p[file->pos++];
            size--;
        }
        return ESUCCESS;
    } else {
        return tmpfs_read(file, buf, size);
    }
}

/***************************************************************************/
/*                                 write()                                 */
/***************************************************************************/

int32_t sysfs_write(file_t *file, void *buf, int32_t size) {
    if ((file->inode->mode & FT_MASK) == FT_REGULAR) {
        return EIO;
    } else {
        return tmpfs_write(file, buf, size);
    }
}

/***************************************************************************/
/*                                 seek()                                  */
/***************************************************************************/

int32_t sysfs_seek(file_t *file, pos_t newpos) {
    if ((file->inode->mode & FT_MASK) == FT_REGULAR) {
        file->info.tmpfs.off = (uint32_t) newpos;
    } else {
        return tmpfs_seek(file, newpos);
    }
}

/***************************************************************************/
/*                                readdir()                                */
/***************************************************************************/

int32_t sysfs_readdir(file_t *file, dirent_t *dirent) {
    return tmpfs_readdir(file, dirent);
}

/***************************************************************************/
/*                                 ioctl()                                 */
/***************************************************************************/

int32_t sysfs_ioctl(file_t *file, int32_t cmd, void *arg) {
    return tmpfs_ioctl(file, cmd, arg);
}

/***************************************************************************/
/*                               fsdriver_t                                */
/***************************************************************************/

fsd_t sysfs_t = {

    /* alias:        */ "sysfs",
    /* flags:        */ 0,

    /* read_super:   */ sysfs_read_super,
    /* write_super:  */ sysfs_write_super,
    /* put_super:    */ sysfs_put_super,
    /* read_inode:   */ sysfs_read_inode,
    /* update_inode: */ sysfs_update_inode,
    /* put_inode:    */ sysfs_put_inode,

    /* lookup:       */ sysfs_lookup,
    /* mknod:        */ sysfs_mknod,
    /* link:         */ sysfs_link,
    /* unlink:       */ sysfs_unlink,
    /* mkdir:        */ sysfs_mkdir,
    /* rmdir:        */ sysfs_rmdir,
    /* truncate:     */ sysfs_truncate,

    /* open:         */ sysfs_open,
    /* release:      */ sysfs_release,
    /* read:         */ sysfs_read,
    /* write:        */ sysfs_write,
    /* seek:         */ sysfs_seek,
    /* readdir:      */ sysfs_readdir,
    /* ioctl:        */ sysfs_ioctl

};


/***************************************************************************/
/*                               sysfs_reg()                               */
/***************************************************************************/

void sysfs_reg(char *name, char *(*read)(uint32_t *size)) {
    /* add to list */
    sysfile_t *cur = sysfiles;
    sysfile_t *sysfile = kmalloc(sizeof(sysfile_t));
    sysfile->name = kmalloc(strlen(name)+1);
    strcpy(sysfile->name, name);
    sysfile->read = read;
    sysfile->ino = 0;
    sysfile->next = NULL;
    if (!sysfiles) {
        sysfiles = sysfile;
    } else {
        while (cur->next) {
            cur = cur->next;
        }
        cur->next = sysfile;
    }
    /* add to filesystem */
    if (sysfs_sb) {
        inode_t *root = (inode_t *) iget(sysfs_sb, sysfs_sb->root_ino);
        inode_t *child;
        tmpfs_mknod(root, sysfile->name, FT_REGULAR, 0);
        tmpfs_lookup(root, sysfile->name, &child);
        sysfile->ino = child->ino;
        iput(child);
        iput(root);
    }
}

/***************************************************************************/
/*                              sysfs_unreg()                              */
/***************************************************************************/

void sysfs_unreg(char *name) {
    /* remove from linkedlist */
    sysfile_t *prev = NULL;
    sysfile_t *cur  = sysfiles;
    while (cur) {
        if (!strcmp(cur->name, name)) {
            if (!prev) {
                sysfiles = cur->next;
            } else {
                prev->next = cur->next;
            }
            kfree(cur->name);
            kfree(cur);
            break;
        }
        cur = cur->next;
    }
    /* remove from filesystem */
    if (sysfs_sb) {
        inode_t *root = (inode_t *) iget(sysfs_sb, sysfs_sb->root_ino);
        tmpfs_unlink(root, name);
        iput(root);
    }
}
