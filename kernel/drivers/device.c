/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Device Management Unit .                         | |
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

/* An interface to manage hardware devices and device drivers.
 * A hardware device is a some piece of hardware that is part of
 * the computer or connected to it. a device is represented
 * using device_t. All devices are represented using a "linked list".
 */

#include <arch/type.h>
#include <sys/error.h>
#include <sys/mm.h>
#include <sys/device.h>

/* Supported Drivers:  */
/* ------------------- */
driver_t *drivers[] = {
    &root_driver,
    &mem_driver,
    &ramdisk_driver,
    &vtty_driver,
    &pstty_driver,
    &ibmpc_driver,
    &isa_driver,
    &pci_driver,
    &i8259_driver,
    &i8253_driver,
    &i8042_driver,
    &ps2_keyboard,
    &ps2_mouse,
    &vga_driver,
    &ide_driver,
    &atadisk_driver,
    &ahci_driver,
    &uhci_driver,
    &usbhub_driver,
    &usbmass_driver,
    &scsidisk_driver,
    &partition_driver
};

/* Data Structures:  */
/* ----------------- */
linkedlist devices;
linkedlist busses;

/* keep track of devid:  */
/* --------------------- */
uint32_t last_devid = 0;

/* ================================================================= */
/*                         devid_to_dev()                            */
/* ================================================================= */

device_t *devid_to_dev(uint32_t devid) {
    /* get the device_t structure that corresponds to "devid". */
    device_t *ptr = (device_t *) devices.first;
    while((ptr != NULL) && (ptr->devid != devid))
        ptr = (device_t *) ptr->next;
    return ptr;
}

/* ================================================================= */
/*                          dev_mkbus()                              */
/* ================================================================= */

uint32_t dev_mkbus(bus_t** ret, uint32_t type, device_t *ctl) {
    /* allocate memory for the bus structure. */
    *ret = (bus_t *) kmalloc(sizeof(bus_t));
    if (*ret == NULL) return ENOMEM;

    /* insert into the linked list: */
    linkedlist_add(&busses, (linknode *) *ret);
    if (ctl != NULL)
        ctl->child_bus = *ret;

    /* initialize: */
    (*ret)->type = type;
    (*ret)->ctl  = ctl;
    return ESUCCESS;
}

/* ================================================================= */
/*                            dev_add()                              */
/* ================================================================= */

uint32_t dev_add(device_t **ret, bus_t* bus, class_t cls,
                 reslist_t resources, void* config) {

    /* this method adds a device to bus, assigns a driver
     * to it, and probes it.
     */
    uint32_t i, j;

    /* allocate memory for device structure. */
    *ret = (device_t *) kmalloc(sizeof(device_t));
    if (*ret == NULL) return ENOMEM;

    /* insert into the linked list: */
    linkedlist_addlast(&devices, *ret);

    /* initialize the structure: */
    (*ret)->devid = ++last_devid;
    (*ret)->parent_bus = bus;
    (*ret)->child_bus = NULL;
    (*ret)->cls = cls;
    (*ret)->resources = resources;
    (*ret)->sb = NULL;

    /* Look up for an appropriate device driver: */
    /* The appropriate device driver is the one that supports
     * device of class "cls".
     */
    for(i = 0; i < DRIVER_COUNT; i++) {
        /* check drivers[i]: */
        for (j = 0; j < drivers[i]->cls_count; j++) {
            uint32_t dbus    = drivers[i]->cls[j].bus;
            uint32_t dbase   = drivers[i]->cls[j].base;
            uint32_t dsub    = drivers[i]->cls[j].sub;
            uint32_t dprogif = drivers[i]->cls[j].progif;
            if ((dbus    == cls.bus   ) &&
                (dbase   == BASE_ANY || dbase   == cls.base  ) &&
                (dsub    == SUB_ANY  || dsub    == cls.sub   ) &&
                (dprogif == IF_ANY   || dprogif == cls.progif))
                    break; /* matches */
        }
        if (j != drivers[i]->cls_count)
            break; /* found */
    }

    /* Probe the driver if found: */
    if (i < DRIVER_COUNT)  {
        (*ret)->driver = drivers[i];
        return drivers[i]->probe(*ret, config);
    }

    /* driver not found, return. */
    (*ret)->driver = NULL;
    return ENODEV;

}

/* ================================================================= */
/*                           dev_read()                              */
/* ================================================================= */

uint32_t dev_read(device_t *dev, uint64_t off, uint32_t size, char *buff) {

    if (dev == NULL)
        return ENOENT;

    if (dev->driver == NULL)
        return ENODEV;

    return dev->driver->read(dev, off, size, buff);

}

/* ================================================================= */
/*                           dev_write()                             */
/* ================================================================= */

uint32_t dev_write(device_t *dev, uint64_t off, uint32_t size, char *buff) {

    if (dev == NULL)
        return ENOENT;

    if (dev->driver == NULL)
        return ENODEV;

    return dev->driver->write(dev, off, size, buff);

}

/* ================================================================= */
/*                           dev_ioctl()                             */
/* ================================================================= */

uint32_t dev_ioctl(device_t *dev, uint32_t cmd, void *data) {

    if (dev == NULL)
        return ENOENT;

    if (dev->driver == NULL)
        return ENODEV;

    return dev->driver->ioctl(dev, cmd, data);

}

/* ================================================================= */
/*                            dev_irq()                              */
/* ================================================================= */

uint32_t dev_irq(device_t *dev, uint32_t irqn) {

    if (dev == NULL)
        return ENOENT;

    if (dev->driver == NULL)
        return ENODEV;

    return dev->driver->irq(dev, irqn);

}

/* ================================================================= */
/*                            get_devices()                          */
/* ================================================================= */

char *get_devices(uint32_t *size) {
    uint32_t count = 0;
    extern linkedlist devices;
    device_t *dev = (device_t *) devices.first;
    char *buf = kmalloc(devices.count*80+1);
    if (!buf)
        return NULL;
    /* loop over devices */
    while (dev) {
        if (dev->driver) {
            count += sputs(&buf[count], dev->driver->alias);
        } else {
            count += sputs(&buf[count], "nodriver");
        }
        count += sputs(&buf[count], " ");
        count += sputd(&buf[count], dev->devid);
        count += sputs(&buf[count], " ");
        count += sputd(&buf[count], dev->cls.bus);
        count += sputs(&buf[count], " ");
        count += sputd(&buf[count], dev->cls.base);
        count += sputs(&buf[count], " ");
        count += sputd(&buf[count], dev->cls.sub);
        count += sputs(&buf[count], " ");
        count += sputd(&buf[count], dev->cls.progif);
        count += sputs(&buf[count], "\n");
        dev = dev->next;
    }
    *size = count;
    /* done */
    return buf;
}

/* ================================================================= */
/*                            dev_init()                             */
/* ================================================================= */

void dev_init() {

    /* Device manager initialization routine. */
    device_t *t;
    class_t cls = {BUS_NOBUS,BASE_NOBUS_ROOT,SUB_ANY,IF_ANY}; /* root class */
    reslist_t reslist = {0, NULL};

    /* print some info */
    printk("\n");
    printk("Starting kernel device management unit...\n");

    /* initialize linked lists: */
    linkedlist_init(&devices);
    linkedlist_init(&busses);

    /* add dev file to sysfs */
    sysfs_reg("dev", get_devices);

    /* load the root driver: */
    dev_add(&t, NULL, cls, reslist, NULL);

}
