/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> USB Subsystem: Hub Device Driver                 | |
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
#include <arch/cache.h>
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
#include <usb/core.h>
#include <usb/hub.h>

/* Prototypes: */
uint32_t usbhub_probe(device_t *, void *);
uint32_t usbhub_read (device_t *, uint64_t, uint32_t, char *);
uint32_t usbhub_write(device_t *, uint64_t, uint32_t, char *);
uint32_t usbhub_ioctl(device_t *, uint32_t, void *);
uint32_t usbhub_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_USB, BASE_USB_HUB, SUB_USB_HUB, IF_ANY}
};

/* driver_t structure that identifies this driver: */
driver_t usbhub_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "usbhub",
    /* probe:     */ usbhub_probe,
    /* read:      */ usbhub_read,
    /* write:     */ usbhub_write,
    /* ioctl:     */ usbhub_ioctl,
    /* irq:       */ usbhub_irq
};

typedef struct info {
    usb_device_t *usbdev;
    int port_count;
} info_t;

/* ================================================================= */
/*                             Interface                             */
/* ================================================================= */

uint32_t usbhub_probe(device_t *dev, void *config) {

    usb_interface_t *usbif = (usb_interface_t *) config;
    usb_device_t *usbdev = usbif->usbdev;
    char buf[100];
    hub_descriptor_t *hubdesc = (hub_descriptor_t *) buf;
    bus_t *usbbus;
    wPortStatus_t *status = (wPortStatus_t *) &buf[0];
    wPortChange_t *change = (wPortChange_t *) &buf[2];
    int i;

    /* create info_t structure: */
    info_t *info = (info_t *) kmalloc(sizeof(info_t));
    dev->drvreg = (uint32_t) info;
    if (info == NULL)
        return ENOMEM; /* i am sorry :D */

    /* inform the user of our progress */
    printk("%aUSB%a: ", 0x0A, 0x0F);
    printk("Hub added with address %d.\n", usbdev->addr);

    /* create hub bus */
    dev_mkbus(&usbbus, BUS_USB, dev);

    /* get hub descriptor */
    usb_control_msg(usbdev,
                    0,
                    GET_DESCRIPTOR,    /* request */
                    0xA0,              /* request type */
                    HUB_DESCRIPTOR<<8, /* value */
                    0x00,              /* index */
                    buf,
                    100,
                    2000);

    /* store information in info structure */
    info->usbdev = usbdev;
    info->port_count = hubdesc->bNbrPorts;

    /* get info about connectivity of the ports */
    for (i = 0; i < info->port_count; i++) {
        /* get port status */
        usb_control_msg(usbdev,
                        0,
                        GET_STATUS,        /* request */
                        0xA3,              /* request type */
                        0,                 /* value */
                        i,                 /* port index */
                        buf,
                        4,
                        2000);
        /* port connected? */
        if (change->c_port_connection) {
            /* clear c_port_connection */
            usb_control_msg(usbdev,
                            0,
                            CLEAR_FEATURE,     /* request */
                            0x23,              /* request type */
                            C_PORT_CONNECTION, /* value */
                            i,                 /* port index */
                            NULL,
                            0,
                            2000);
            /* check action type */
            if (status->port_connection) {
                /* there is a device connected to this port,
                * add it to the system!
                */
                usb_enum_bus(usbif, i, status->port_low_speed);
            } else {
                /* device disconnected */
            }
        }
    }

    /* done */
    return ESUCCESS;
}

uint32_t usbhub_read(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    return ESUCCESS;
}

uint32_t usbhub_write(device_t *dev, uint64_t off, uint32_t size,char *buff) {
    return ESUCCESS;
}

uint32_t usbhub_ioctl(device_t *dev, uint32_t cmd, void *data) {
    return ESUCCESS;
}

uint32_t usbhub_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
