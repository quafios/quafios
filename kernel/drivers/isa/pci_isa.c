/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> PCI to ISA Bridge Device Driver.                 | |
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
#include <sys/scheduler.h>
#include <serio/8042kbc.h>

/* Prototypes: */
uint32_t pci_isa_probe(device_t *, void *);
uint32_t pci_isa_read (device_t *, uint64_t, uint32_t, char *);
uint32_t pci_isa_write(device_t *, uint64_t, uint32_t, char *);
uint32_t pci_isa_ioctl(device_t *, uint32_t, void *);
uint32_t pci_isa_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_PCI, BASE_PCI_BRIDGE, SUB_PCI_BRIDGE_PCI_TO_ISA, IF_ANY}
};

/* driver_t structure that identifies this driver: */
driver_t isa_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "pci_isa",
    /* probe:     */ pci_isa_probe,
    /* read:      */ pci_isa_read,
    /* write:     */ pci_isa_write,
    /* ioctl:     */ pci_isa_ioctl,
    /* irq:       */ pci_isa_irq
};

/* ================================================================= */
/*                            Interface                              */
/* ================================================================= */

uint32_t pci_isa_probe(device_t *dev, void *config) {

    bus_t *isabus;
    device_t *t;
    class_t cls;
    reslist_t reslist;
    resource_t *resv; /* vector of resources. */
    i8042_init_t i8042_config;

    /* Create the isa bus, this allows legacy support for
     * all the old standard system bus of IBM PC, and for
     * most devices appear on it.
     */
    dev_mkbus(&isabus, BUS_ISA, NULL);

    /* Add legacy devices of IBM PC that were not added
     * by the host machine driver.
     */

    /* Intel's 8042 Peripheral Controller:
     * -------------------------------------
     * i8042 is a programmable peripheral controller developed
     * by intel. it is found on PC/AT machines supporting PS/2
     * keyboards. In modern IBM PC compatibles, it is loaded
     * with some piece of firmware that allows communication
     * with PS/2 keyboard and PS/2 mouse, and provides System
     * Reset service, and enabling/disabling A20 gate.
     * In Quafios, this piece of firmware is gonna be called
     * "IBM ATKBC" because its main aim is to control AT PS/2
     * keyboard.
     */
    /* class code: */
    cls.bus    = BUS_ISA;
    cls.base   = BASE_ISA_INTEL;
    cls.sub    = SUB_ISA_INTEL_8042;
    cls.progif = IF_ANY;
    /* resources: */
    resv = (resource_t *) kmalloc(sizeof(resource_t)*4);
    if (resv == NULL) return ENOMEM;
    resv[0].type = RESOURCE_TYPE_PORT;
    resv[0].data.port.base = 0x60;      /* standard in IBM PC. */
    resv[0].data.port.size = 1;         /* 1 byte (data). */
    resv[1].type = RESOURCE_TYPE_PORT;
    resv[1].data.port.base = 0x64;      /* standard. */
    resv[1].data.port.size = 1;         /* 1 byte (command). */
    resv[2].type = RESOURCE_TYPE_IRQ;
    resv[2].data.irq.number = 1;        /* IRQ1 (PS/2 Keyboard). */
    resv[3].type = RESOURCE_TYPE_IRQ;
    resv[3].data.irq.number = 12;       /* IRQC (PS/2 Mouse). */
    /* resource list: */
    reslist.count = 4;
    reslist.list  = resv;
    /* config: */
    i8042_config.firmware_code = i8042_ATKBC;
    /* now add the 8042: */
    dev_add(&t, isabus, cls, reslist, &i8042_config);

    /* finally... */
    return ESUCCESS;
}

uint32_t pci_isa_read(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    return ESUCCESS;
}

uint32_t pci_isa_write(device_t *dev, uint64_t off, uint32_t size, char *buff){
    return ESUCCESS;
}

uint32_t pci_isa_ioctl(device_t* dev, uint32_t c, void *data) {
    return ESUCCESS;
}

uint32_t pci_isa_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
