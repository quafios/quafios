/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> RAM disk Device Driver.                          | |
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
#include <mem/ramdisk.h>

/* Prototypes: */
uint32_t ramdisk_probe(device_t *, void *);
uint32_t ramdisk_read (device_t *, uint64_t, uint32_t, char *);
uint32_t ramdisk_write(device_t *, uint64_t, uint32_t, char *);
uint32_t ramdisk_ioctl(device_t *, uint32_t, void *);
uint32_t ramdisk_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_MEMORY, BASE_MEMORY_NODE, SUB_MEMORY_NODE_RAMDISK, IF_ANY}
};

/* driver_t structure that identifies this driver: */
driver_t ramdisk_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "ramdisk",
    /* probe:     */ ramdisk_probe,
    /* read:      */ ramdisk_read,
    /* write:     */ ramdisk_write,
    /* ioctl:     */ ramdisk_ioctl,
    /* irq:       */ ramdisk_irq
};

typedef struct {
    uint32_t base;
    uint32_t size;
} info_t;

/* ================================================================= */
/*                            Interface                              */
/* ================================================================= */

uint32_t ramdisk_probe(device_t *dev, void *config) {
    ramdisk_init_t *cfg = config;
    extern uint32_t bootdisk;

    info_t *info = (info_t *) kmalloc(sizeof(info_t));
    dev->drvreg = (uint32_t) info;
    if (info == NULL)
            return ENOMEM; /* i am sorry :D */

    info->base = cfg->base;
    info->size = cfg->size;

    bootdisk = dev->devid;

    return ESUCCESS;
}

uint32_t ramdisk_read(device_t *dev, uint64_t off, uint32_t size, char *buff) {

    info_t *info = (info_t *) dev->drvreg;
    if (info == NULL)
        return ENOMEM;

    while (size--) {
        if (off > info->size)
            return ESUCCESS;
        *buff = pmem_readb((uint32_t)(info->base + off++));
        buff++;
    }

    return ESUCCESS;
}

uint32_t ramdisk_write(device_t *dev, uint64_t off, uint32_t size, char *buff){

    info_t *info = (info_t *) dev->drvreg;
    if (info == NULL)
        return ENOMEM;

    while (size--) {
        if (off > info->size)
            return ESUCCESS;
        pmem_writeb((uint32_t) (info->base + off++), *buff);
        buff++;
    }

    return ESUCCESS;
}

uint32_t ramdisk_ioctl(device_t *dev, uint32_t cmd, void *data) {
    return ESUCCESS;
}

uint32_t ramdisk_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
