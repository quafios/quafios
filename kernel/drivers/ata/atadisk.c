/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> ATA Hard Disk Drive Device Driver.               | |
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
#include <lib/linkedlist.h>
#include <sys/error.h>
#include <sys/printk.h>
#include <sys/mm.h>
#include <sys/class.h>
#include <sys/resource.h>
#include <sys/device.h>
#include <sys/bootinfo.h>
#include <sys/scheduler.h>
#include <pci/pci.h>
#include <ata/ide.h>

/* Prototypes: */
uint32_t atadisk_probe(device_t *, void *);
uint32_t atadisk_read (device_t *, uint64_t, uint32_t, char *);
uint32_t atadisk_write(device_t *, uint64_t, uint32_t, char *);
uint32_t atadisk_ioctl(device_t *, uint32_t, void *);
uint32_t atadisk_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_IDE, BASE_ATA_DISK, SUB_ATA_DISK_GENERIC, IF_ANY}
};

/* driver_t structure that identifies this driver: */
driver_t atadisk_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "atadisk",
    /* probe:     */ atadisk_probe,
    /* read:      */ atadisk_read,
    /* write:     */ atadisk_write,
    /* ioctl:     */ atadisk_ioctl,
    /* irq:       */ atadisk_irq
};

typedef struct {
    device_t *dev;
    ata_drive_t *drive;
    uint32_t cache_lock;
    uint64_t cache_sect;
    char     cache_data[512];
} info_t;

/* ================================================================= */
/*                            ATA Interface                          */
/* ================================================================= */

static int32_t read_sectors(info_t *info,
                            uint32_t seccount,
                            uint64_t lba,
                            char *buf) {
    ata_req_t req;
    req.protocol  = ATA_PROTO_PIO;
    req.channel   = info->drive->channel;
    req.drvnum    = info->drive->drvnum;
    req.seccount  = seccount;
    req.lba       = lba;
    req.amode     = info->drive->mode;
    req.cmd       = req.amode == ATA_AMODE_LBA48 ? ATA_CMD_READ_PIO_EXT:
                                                   ATA_CMD_READ_PIO;
    req.buf       = (uint8_t *) buf;
    req.bufsize   = seccount*512;
    req.drqsize   = 512;
    req.direction = ATA_DIR_READ;
    req.wmode     = ATA_WMODE_IRQ;
    dev_ioctl(info->dev->parent_bus->ctl, 0, &req);
    return 0;
}

static int32_t read_sector_part(info_t *info,
                                uint64_t lba,
                                uint32_t off,
                                uint32_t size,
                                char *buf) {
    int32_t err, i;
    while(info->cache_lock);
    info->cache_lock = 1;
    if (info->cache_sect != lba) {
        if (err = read_sectors(info, 1, lba, info->cache_data))
            return err;
    }
    for (i = off; i < off+size; i++)
        *buf++ = info->cache_data[i];
    info->cache_lock = 0;
    return 0;
}

/* ================================================================= */
/*                             Interface                             */
/* ================================================================= */

uint32_t atadisk_probe(device_t *dev, void *drive_ptr) {

    /* create info_t structure: */
    info_t *info = (info_t *) kmalloc(sizeof(info_t));
    dev->drvreg = (uint32_t) info;
    if (info == NULL)
        return ENOMEM; /* i am sorry :D */

    /* splash */
    printk("- ATA DISK DRIVER (%d)\n", dev->devid);

    /* store drive pointer in info structure */
    info->drive = drive_ptr;

    /* store device pointer in info structure */
    info->dev = dev;

    /* initialize buffer */
    info->cache_lock = 0;
    info->cache_sect = -1;

    /* scan for partitions */
    partprobe(dev);

    /* done */
    return ESUCCESS;

}

uint32_t atadisk_read(device_t *dev, uint64_t off, uint32_t size, char *buff){
    info_t *info = (info_t *) dev->drvreg;
    ata_drive_t *drive = info->drive;
    uint32_t max_seccount = drive->mode == ATA_AMODE_LBA48 ? 0x10000 : 0x100;

    /* head */
    if (off % 512) {
        /* offset is not sector aligned */
        uint32_t rem = (off+512)/512 - off;
        if (size <= rem) {
            if (read_sector_part(info, off/512, off%512, size, buff))
                return EIO;
            buff += size;
            size = 0;
        } else {
            if (read_sector_part(info, off/512, off%512, rem, buff))
                return EIO;
            buff += rem;
            size -= rem;
        }
    }

    /* body */
    while (size >= 512) {
        /* read all remaining sectors */
        uint32_t sects = size/512;
        if (sects >= max_seccount) {
            if (read_sectors(info, max_seccount, off/512, buff))
                return EIO;
            size -= max_seccount*512;
            buff += max_seccount*512;
        } else {
            if (read_sectors(info, sects, off/512, buff))
                return EIO;
            size -= sects*512;
            buff += sects*512;
        }
    }

    /* tail*/
    if (size) {
        if (read_sector_part(info, off/512, 0, size, buff))
            return EIO;
    }

    /* done */
    return ESUCCESS;
}

uint32_t atadisk_write(device_t *dev, uint64_t off, uint32_t size,char *buff){
    return ESUCCESS;
}

uint32_t atadisk_ioctl(device_t *dev, uint32_t cmd, void *data) {
    return ESUCCESS;
}

uint32_t atadisk_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
