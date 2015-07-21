/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Generic Partition Device Driver.                 | |
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
#include <storage/disk.h>

/* Prototypes: */
uint32_t partition_probe(device_t *, void *);
uint32_t partition_read (device_t *, uint64_t, uint32_t, char *);
uint32_t partition_write(device_t *, uint64_t, uint32_t, char *);
uint32_t partition_ioctl(device_t *, uint32_t, void *);
uint32_t partition_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_DISK, BASE_DISK_PARTITION, SUB_ANY, IF_ANY}
};

/* driver_t structure that identifies this driver: */
driver_t partition_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "partition",
    /* probe:     */ partition_probe,
    /* read:      */ partition_read,
    /* write:     */ partition_write,
    /* ioctl:     */ partition_ioctl,
    /* irq:       */ partition_irq
};

typedef struct {
    device_t *diskdev;
    uint64_t  base;
    disk_t   *disk;
    char     *name;
} info_t;

/* ================================================================= */
/*                             Interface                             */
/* ================================================================= */

uint32_t partition_probe(device_t *dev, void *config) {

    /* create info_t structure: */
    info_t *info = (info_t *) kmalloc(sizeof(info_t));
    dev->drvreg = (uint32_t) info;
    if (info == NULL)
        return ENOMEM; /* i am sorry :D */

    /* set info_t structure */
    info->diskdev = (device_t *) devid_to_dev(dev->cls.progif);
    info->base = *((uint64_t *) config) * 512;
    info->disk = (disk_t *) get_disk_by_devid(info->diskdev->devid);
    info->name = kmalloc(100);

    /* register at devfs */
    if (info->disk) {
        strcpy(info->name, info->disk->name);
        itoa(dev->cls.sub, &info->name[strlen(info->disk->name)], 10);
        devfs_reg(info->name, dev->devid);
    }

    /* done */
    return ESUCCESS;
}

uint32_t partition_read(device_t *dev, uint64_t off, uint32_t size,char *buff){
    info_t *info = (info_t *) dev->drvreg;
    return dev_read(info->diskdev, info->base+off, size, buff);
}

uint32_t partition_write(device_t *dev, uint64_t off,uint32_t size,char *buff){
    info_t *info = (info_t *) dev->drvreg;
    return dev_write(info->diskdev, info->base+off, size, buff);
}

uint32_t partition_ioctl(device_t *dev, uint32_t cmd, void *data) {
    return ESUCCESS;
}

uint32_t partition_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
