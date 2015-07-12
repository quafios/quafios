/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> Filesystem: Super block operations.              | |
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
#include <sys/mm.h>
#include <sys/fs.h>

/* Supported Filesystems:  */
/* ----------------------- */
fsd_t *fsdrivers[] = {
    &tmpfs_t,
    &devfs_t,
    &sysfs_t,
    &diskfs_t
};

/* root mount point: */
vfsmount_t *vfsroot = NULL;

/***************************************************************************/
/*                                 fsd()                                   */
/***************************************************************************/

fsd_t *fsd(char* alias) {

    /* search for filesystem driver that associates "alias". */
    int32_t i;
    fsd_t *ret = NULL;
    for(i = 0; i < FSDRIVER_COUNT; i++)
        if (!strcmp(fsdrivers[i]->alias, alias)) {
            ret = fsdrivers[i];
            break;
        }

    return ret;
}

/***************************************************************************/
/*                                mount()                                  */
/***************************************************************************/

int32_t mount(char *devfile, char *mntpoint, char *fstype,
      uint32_t flags, void *data) {

    /* mount() System Call. */
    uint32_t error;
    vfsmount_t *vfsmount = NULL;
    vfsmount_t *vfsparent = NULL;
    inode_t *mp_inode = NULL;
    super_block_t *sb = NULL;
    device_t *dev = NULL;
    fsd_t *fsdriver;
    namei_t namei_data;
    int32_t err;
    inode_t *i;

    /* I) Process "fstype" argument:  */
    /* ------------------------------ */
    /* fstype must be valid: */
    fsdriver = fsd(fstype);
    if (fsdriver == NULL)
        return -ENODEV;

    /* II) Get Disk Device Driver:  */
    /* ---------------------------- */
    if (fsdriver->flags & FSD_REQDEV) {

        /* get the inode of the device file: */
        namei_data;
        err = namei(NULL, devfile, &namei_data);

        /* exists? */
        if (err)
            return -ENODEV;
        kfree(namei_data.path); /* not needed now. */

        /* device file? */
        i = namei_data.inode;
        if ((i->mode & FT_MASK) != FT_SPECIAL) {
            iput(i);
            return -EINVAL;
        }

        /* get device structure: */
        dev = (device_t *) devid_to_dev(i->devid);

        /* release the inode: */
        iput(i);

    }

    /* III) Validate the mount point:  */
    /* ------------------------------- */
    if (vfsroot != NULL) {

        /* get the inode of the mount point: */
        namei_t namei_data;
        int32_t err;
        if (err = namei(NULL, mntpoint, &namei_data))
            return -err;

        /* extract data: */
        mp_inode = namei_data.inode;
        vfsparent = namei_data.mp;
        kfree(namei_data.path); /* not needed now. */

        /* must be directory: */
        if ((mp_inode->mode & FT_MASK) != FT_DIR) {
            iput(mp_inode);
            return -ENOTDIR;
        }

    }

    /* IV) Allocate a mount point structure:  */
    /* -------------------------------------- */
    vfsmount = kmalloc(sizeof(vfsmount_t));
    if (!vfsmount) {
        if (mp_inode) {
            iput(mp_inode);
        }
        return -ENOMEM;
    }

    /* V) Read the super block:  */
    /* ------------------------- */
    if (!dev || !(dev->sb)) {
        /* first mount.. */
        sb = fsdriver->read_super(dev);

        /* error? */
        if (!sb) {
            kfree(vfsmount);
            if (mp_inode) {
                iput(mp_inode);
            }
            return -ENOMEM;
        }

        /* update device structure: */
        if (dev)
            dev->sb = sb;
    } else {
        /* mounted before. */
        sb = dev->sb;
        sb->mounts++;
    }

    /* VI) Initialize the mount point structure:  */
    /* ------------------------------------------ */
    vfsmount->vfsparent = vfsparent;
    vfsmount->mp_inode  = mp_inode;
    vfsmount->sb        = sb;

    /* VII) Inform the parent filesystem of this mount:  */
    /* ------------------------------------------------- */
    if (mp_inode) {
        mp_inode->submount = vfsmount;
    } else {
        /* first mount! no parent fs. */
        vfsroot = vfsmount;
    }

    /* VIII) Return:  */
    /* -------------- */
    return ESUCCESS;

}

/***************************************************************************/
/*                               umount()                                  */
/***************************************************************************/

int32_t umount(char *target) {

    namei_t namei_data;
    inode_t *inode;
    vfsmount_t *vfsmount = NULL;
    super_block_t *sb;

    /* get the inode of `target': */
    int32_t err;
    if (err = namei(NULL, target, &namei_data))
        return -err;

    /* extract data: */
    inode = namei_data.inode;
    vfsmount = namei_data.mp;
    kfree(namei_data.path); /* not needed now. */

    /* root directory? */
    if (inode->ino != inode->sb->root_ino) {
        iput(inode);
        return -EINVAL;
    }

    /* get super block: */
    sb = inode->sb;
    iput(inode);

    /* put super block: */
    if (sb->mounts > 1) {
        sb->mounts--;
    } else if (sb->icount) {
        return -EBUSY;
    } else {
        if (sb->dev)
            sb->dev->sb = NULL;
        sb->fsdriver->put_super(sb);
    }

    /* drop the mount point structures. */
    if (vfsmount->mp_inode) {
        vfsmount->mp_inode->submount = NULL;
        iput(vfsmount->mp_inode);
    } else {
        vfsroot = NULL;
    }
    kfree(vfsmount);

    /* done: */
    return 0;

}
