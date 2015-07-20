/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> SCSI Hard Disk Drive Device Driver.              | |
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
#include <scsi/scsi.h>

/* Prototypes: */
uint32_t scsidisk_probe(device_t *, void *);
uint32_t scsidisk_read (device_t *, uint64_t, uint32_t, char *);
uint32_t scsidisk_write(device_t *, uint64_t, uint32_t, char *);
uint32_t scsidisk_ioctl(device_t *, uint32_t, void *);
uint32_t scsidisk_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_SCSI, BASE_SCSI_DISK, SUB_SCSI_DISK_GENERIC, IF_ANY}
};

/* driver_t structure that identifies this driver: */
driver_t scsidisk_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "scsidisk",
    /* probe:     */ scsidisk_probe,
    /* read:      */ scsidisk_read,
    /* write:     */ scsidisk_write,
    /* ioctl:     */ scsidisk_ioctl,
    /* irq:       */ scsidisk_irq
};

typedef struct {
    device_t *dev;
    device_t *ctrlr;
    int32_t   lun;
    uint32_t  cache_lock;
    uint64_t  cache_sect;
    char      cache_data[512];
} info_t;

/* ================================================================= */
/*                           SCSI Interface                          */
/* ================================================================= */

static int32_t read_sectors(info_t *info,
                            uint32_t seccount,
                            uint64_t lba,
                            char *buf) {

    uint8_t cdb[16];
    scsi_cmd_t scsi_cmd;
    int32_t retval, i;

    /* construct CDB */
    cdb[ 0] = 0x28; /* READ (10) */
    cdb[ 1] = 0x00;
    cdb[ 2] = (lba>>24)&0xFF;
    cdb[ 3] = (lba>>16)&0xFF;
    cdb[ 4] = (lba>> 8)&0xFF;
    cdb[ 5] = (lba>> 0)&0xFF;
    cdb[ 6] = 0x00;
    cdb[ 7] = (seccount>> 8)&0xFF;
    cdb[ 8] = (seccount>> 0)&0xFF;
    cdb[ 9] = 0x00;

    /* send command to SCSI controller */
    scsi_cmd.lun = info->lun;
    scsi_cmd.cdb = cdb;
    scsi_cmd.cdb_len = 10;
    scsi_cmd.data = buf;
    scsi_cmd.data_len = seccount*512;
    scsi_cmd.is_write = 0;
    retval = dev_ioctl(info->ctrlr, 0, &scsi_cmd);

    /* done */
    return retval;

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

uint32_t scsidisk_probe(device_t *dev, void *config) {

    scsi_config_t *scsi_config = (scsi_config_t *) config;

    /* create info_t structure: */
    info_t *info = (info_t *) kmalloc(sizeof(info_t));
    dev->drvreg = (uint32_t) info;
    if (info == NULL)
        return ENOMEM; /* i am sorry :D */

    /* splash */
    printk("- SCSI DISK DRIVER (%d)\n", dev->devid);

    /* initialize info structure */
    info->ctrlr      = scsi_config->ctrlr;
    info->lun        = scsi_config->lun;
    info->cache_lock = 0;
    info->cache_sect = -1;

    /* scan for partitions */
    partprobe(dev);

    /* done */
    return ESUCCESS;

}

uint32_t scsidisk_read(device_t *dev, uint64_t off, uint32_t size, char *buff){

    info_t *info = (info_t *) dev->drvreg;
    uint32_t max_seccount = 0x10000;

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

uint32_t scsidisk_write(device_t *dev, uint64_t off, uint32_t size,char *buff){
    return ESUCCESS;
}

uint32_t scsidisk_ioctl(device_t *dev, uint32_t cmd, void *data) {
    return ESUCCESS;
}

uint32_t scsidisk_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
