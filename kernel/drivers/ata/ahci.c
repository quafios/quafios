/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> AHCI Device Driver.                              | |
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

/* Prototypes: */
uint32_t ahci_probe(device_t *, void *);
uint32_t ahci_read (device_t *, uint64_t, uint32_t, char *);
uint32_t ahci_write(device_t *, uint64_t, uint32_t, char *);
uint32_t ahci_ioctl(device_t *, uint32_t, void *);
uint32_t ahci_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_PCI, BASE_PCI_STORAGE, SUB_PCI_STORAGE_SATA, IF_PCI_STORAGE_SATA_AHCI}
};

/* driver_t structure that identifies this driver: */
driver_t ahci_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "ahci",
    /* probe:     */ ahci_probe,
    /* read:      */ ahci_read,
    /* write:     */ ahci_write,
    /* ioctl:     */ ahci_ioctl,
    /* irq:       */ ahci_irq
};

typedef struct {
    /* pci_info */
    uint8_t bus;
    uint8_t devno;
    uint8_t func;
    device_t *master;
    /* ahci info */
} info_t;

/* ================================================================= */
/*                             Interface                             */
/* ================================================================= */

uint32_t ahci_probe(device_t *dev, void *config) {
    int i;
    pci_config_t *pci_config = config;

    /* create info_t structure: */
    info_t *info = (info_t *) kmalloc(sizeof(info_t));
    dev->drvreg = (uint32_t) info;
    if (info == NULL)
        return ENOMEM; /* i am sorry :D */

    /* debug info */
    for (i = 0; i < dev->resources.count; i++) {
        printk("type: %d, base: %x\n", dev->resources.list[i]);
    }

    /* print message */
    printk("AHCI initialization.\n");

    /* done */
    return ESUCCESS;
}

uint32_t ahci_read(device_t *dev, uint64_t off, uint32_t size, char *buff){
    return ESUCCESS;
}

uint32_t ahci_write(device_t *dev, uint64_t off, uint32_t size,char *buff){
    return ESUCCESS;
}

uint32_t ahci_ioctl(device_t *dev, uint32_t cmd, void *data) {
    return ESUCCESS;
}

uint32_t ahci_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
