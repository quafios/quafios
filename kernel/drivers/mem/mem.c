/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Memory Pseudo Device Driver.                     | |
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
#include <mem/ramdisk.h>

extern bootinfo_t *bootinfo;

/* Prototypes: */
uint32_t mem_probe(device_t *, void *);
uint32_t mem_read (device_t *, uint64_t, uint32_t, char *);
uint32_t mem_write(device_t *, uint64_t, uint32_t, char *);
uint32_t mem_ioctl(device_t *, uint32_t, void *);
uint32_t mem_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_GENESIS, BASE_GENESIS_MEMORY, SUB_GENESIS_MEMORY, IF_ANY}
};

/* driver_t structure that identifies this driver: */
driver_t mem_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "mem",
    /* probe:     */ mem_probe,
    /* read:      */ mem_read,
    /* write:     */ mem_write,
    /* ioctl:     */ mem_ioctl,
    /* irq:       */ mem_irq
};

/* ================================================================= */
/*                            Interface                              */
/* ================================================================= */

uint32_t mem_probe(device_t* dev, void* config) {
    bus_t *membus;
    device_t *t;
    class_t cls = {BUS_MEMORY, BASE_MEMORY_NODE, SUB_MEMORY_NODE_RAMDISK};
    reslist_t reslist = {0, NULL};
    ramdisk_init_t data;

    /* Create pseudo memory bus: */
    dev_mkbus(&membus, BUS_MEMORY, dev);

    /* Check for live ram disk! */
    if (bootinfo->live) {
        data.base = bootinfo->res[BI_RAMDISK].base;
        data.size = bootinfo->res[BI_RAMDISK].end -
                    bootinfo->res[BI_RAMDISK].base;
        dev_add(&t, membus, cls, reslist, (void *) &data);
    }

    /* done */
    return ESUCCESS;
}

uint32_t mem_read(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    return ESUCCESS;
}

uint32_t mem_write(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    return ESUCCESS;
}

uint32_t mem_ioctl(device_t *dev, uint32_t cmd, void *data) {
    return ESUCCESS;
}

uint32_t mem_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
