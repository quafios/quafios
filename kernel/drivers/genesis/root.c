/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> Root Device Driver.                              | |
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
#include <lib/linkedlist.h>
#include <sys/error.h>
#include <sys/printk.h>
#include <sys/mm.h>
#include <sys/class.h>
#include <sys/resource.h>
#include <sys/device.h>

/* Prototypes: */
uint32_t root_probe(device_t *, void *);
uint32_t root_read (device_t *, uint64_t, uint32_t, char *);
uint32_t root_write(device_t *, uint64_t, uint32_t, char *);
uint32_t root_ioctl(device_t *, uint32_t, void *);
uint32_t root_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_NOBUS, BASE_NOBUS_ROOT, SUB_ANY, IF_ANY}
};

/* driver_t structure that identifies this driver: */
driver_t root_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "root",
    /* probe:     */ root_probe,
    /* read:      */ root_read,
    /* write:     */ root_write,
    /* ioctl:     */ root_ioctl,
    /* irq:       */ root_irq
};

/* ================================================================= */
/*                            Interface                              */
/* ================================================================= */

uint32_t root_probe(device_t* dev, void* config) {
    bus_t *genesis;
    device_t *t;
    class_t cls;
    reslist_t reslist = {0, NULL};

    /* Create genesis bus: */
    dev_mkbus(&genesis, BUS_GENESIS, dev);

    /* Add the "memory" device driver: */
    cls.bus    = BUS_GENESIS;
    cls.base   = BASE_GENESIS_MEMORY;
    cls.sub    = SUB_GENESIS_MEMORY;
    cls.progif = IF_ANY;
    dev_add(&t, genesis, cls, reslist, NULL);

    /* Add a "tty" device driver to act as a system console: */
    cls.bus    = BUS_GENESIS;
    cls.base   = BASE_GENESIS_TTY;
    cls.sub    = SUB_GENESIS_TTY_VIRTUAL;
    cls.progif = IF_ANY;
    dev_add(&t, genesis, cls, reslist, NULL);

    /* Add the "host" device driver: */
    cls.bus    = BUS_GENESIS;
    cls.base   = BASE_GENESIS_MACHINE;
    cls.sub    = SUB_GENESIS_MACHINE_IBMPC;
    cls.progif = IF_ANY;
    dev_add(&t, genesis, cls, reslist, NULL);

    return ESUCCESS;
}

uint32_t root_read(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    return ESUCCESS;
}

uint32_t root_write(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    return ESUCCESS;
}

uint32_t root_ioctl(device_t *dev, uint32_t cmd, void *data) {
    return ESUCCESS;
}

uint32_t root_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
