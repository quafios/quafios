/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Device management header.                        | |
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

#ifndef DEVICE_H
#define DEVICE_H

#include <arch/type.h>
#include <sys/class.h>
#include <lib/linkedlist.h>
#include <sys/resource.h>

/* Type definitions:  */
/* ------------------ */
typedef struct device_str device_t;
typedef struct driver_str driver_t;

/* Driver Structure:  */
/* ------------------ */
struct driver_str {
    uint32_t cls_count; /* count of supported device classes.      */
    class_t* cls;       /* classes of devices this driver can run. */
    char*    alias;     /* alias (String).                         */
    uint32_t (*probe)(device_t *dev, void *config);
    uint32_t (*read )(device_t *dev, uint64_t off, uint32_t size, char *buff);
    uint32_t (*write)(device_t *dev, uint64_t off, uint32_t size, char *buff);
    uint32_t (*ioctl)(device_t *dev, uint32_t cmd, void *data);
    uint32_t (*irq  )(device_t *dev, uint32_t irqn);
};

/* Supported Drivers:  */
/* ------------------- */
extern driver_t root_driver;
extern driver_t mem_driver;
extern driver_t ramdisk_driver;
extern driver_t ps2_keyboard;
extern driver_t ps2_mouse;
extern driver_t ibmpc_driver;
extern driver_t isa_driver;
extern driver_t pci_driver;
extern driver_t i8253_driver;
extern driver_t i8042_driver;
extern driver_t i8259_driver;
extern driver_t vtty_driver;
extern driver_t pstty_driver;
extern driver_t vga_driver;
extern driver_t ide_driver;
extern driver_t atadisk_driver;
extern driver_t ahci_driver;
extern driver_t uhci_driver;
extern driver_t usbhub_driver;
extern driver_t usbmass_driver;
extern driver_t scsidisk_driver;
extern driver_t partition_driver;
extern driver_t *drivers[];
#define DRIVER_COUNT    (sizeof(drivers)/sizeof(driver_t*))

/* Bus Structure:  */
/* --------------- */
typedef struct {
    void*        next;      /* next bus in the linkedlist. */
    uint32_t     type;      /* bus class type.             */
    struct device_str *ctl; /* Controller Device.          */
} bus_t;

/* Bus Linked List:  */
/* ----------------- */
#ifdef QUAFIOS_KERNEL
extern linkedlist busses;
#endif

/* Device Structure:  */
/* ------------------ */
struct device_str {
    void*               next;       /* next node in the linkedlist.          */
    uint32_t            devid;      /* devid of the device.                  */
    bus_t*              parent_bus; /* the bus to which this device belongs. */
    bus_t*              child_bus;  /* sub bus this device controls.         */
    class_t             cls;        /* The class of this device.             */
    driver_t*           driver;     /* The device driver.                    */
    reslist_t           resources;  /* Resources list.                       */
    struct super_block* sb;         /* filesystem super block.               */
    uint32_t            drvreg;     /* driver related value.                 */
};

/* Device Driver List:  */
/* -------------------- */
#ifdef QUAFIOS_KERNEL
extern linkedlist devices;
#endif

/* Prototypes:  */
/* ------------ */
uint32_t dev_read(device_t *,  uint64_t, uint32_t, char *);
uint32_t dev_write(device_t *, uint64_t, uint32_t, char *);

#endif
