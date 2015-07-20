/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> PCI Bus Device Driver                            | |
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
#include <pci/pci.h>

/* Prototypes: */
uint32_t pci_probe(device_t *, void *);
uint32_t pci_read (device_t *, uint64_t, uint32_t, char *);
uint32_t pci_write(device_t *, uint64_t, uint32_t, char *);
uint32_t pci_ioctl(device_t *, uint32_t, void *);
uint32_t pci_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_HOST, BASE_HOST_SUBSYSTEM, SUB_HOST_SUBSYSTEM_PCI, IF_ANY}
};

/* driver_t structure that identifies this driver: */
driver_t pci_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "pci",
    /* probe:     */ pci_probe,
    /* read:      */ pci_read,
    /* write:     */ pci_write,
    /* ioctl:     */ pci_ioctl,
    /* irq:       */ pci_irq
};

/* ================================================================= */
/*                   Configuration Space Access                      */
/* ================================================================= */

/* ports */
#define PCI_PORT_INDEX          0 /* Index Port */
#define PCI_PORT_DATA           4 /* Data Port  */

/* configuration space registers */
#define PCI_REG_VENDOR_ID       0x00
#define PCI_REG_DEVICE_ID       0x02
#define PCI_REG_COMMAND         0x04
#define PCI_REG_STATUS          0x06
#define PCI_REG_REVISION_ID     0x08
#define PCI_REG_PROG_IF         0x09
#define PCI_REG_SUBCLASS        0x0A
#define PCI_REG_CLASS           0x0B
#define PCI_REG_CACHE_LINE_SIZE 0x0C
#define PCI_REG_LATENCY_TIMER   0x0D
#define PCI_REG_HEADER_TYPE     0x0E
#define PCI_REG_BIST            0x0F
#define PCI_REG_BASE_ADDRESS_0  0x10
#define PCI_REG_BASE_ADDRESS_1  0x14
#define PCI_REG_BASE_ADDRESS_2  0x18
#define PCI_REG_BASE_ADDRESS_3  0x1C
#define PCI_REG_BASE_ADDRESS_4  0x20
#define PCI_REG_BASE_ADDRESS_5  0x24
#define PCI_REG_CARDBUS_CIS_PTR 0x28
#define PCI_REG_SUBSYS_VENDOR   0x2C
#define PCI_REG_SUBSYS_ID       0x2E
#define PCI_REG_EXPANSION_ROM   0x30
#define PCI_REG_CAP_PTR         0x34
#define PCI_REG_RESERVED_1      0x35
#define PCI_REG_RESERVED_4      0x38
#define PCI_REG_INTERRUPT_LINE  0x3C
#define PCI_REG_INTERRUPT_PIN   0x3D
#define PCI_REG_MIN_GRANT       0x3E
#define PCI_REG_MAX_LATENCY     0x3F

static uint32_t port_read(device_t *dev, uint32_t port) {
    uint32_t base = dev->resources.list[0].data.port.base;
    uint32_t type = dev->resources.list[0].type;
    return ioread(4, type, base, port);
}

static void port_write(device_t *dev, uint32_t port, uint32_t data) {
    uint32_t base = dev->resources.list[0].data.port.base;
    uint32_t type = dev->resources.list[0].type;
    iowrite(4, type, data, base, port);
}

static uint32_t get_index(uint32_t bus,
                          uint32_t devno,
                          uint32_t func,
                          uint32_t reg) {
    /* bits  7: 0 = register number (0-255)
     * bits 10: 8 = function number (0-7)
     * bits 15:11 = device number (0-31)
     * bits 23:16 = bus number (0-255)
     * bits 30:24 = 0 - reserved
     * bit     31 = 1 - this bit is always set for a PCI access.
     */
    return (1<<31)|((bus&255)<<16)|((devno&31)<<11)|((func&7)<<8)|(reg&255);
}

static uint8_t read_conf8(device_t* dev,
                         uint32_t bus,
                         uint32_t devno,
                         uint32_t func,
                         uint32_t reg) {
    uint32_t dword;
    port_write(dev, PCI_PORT_INDEX, get_index(bus, devno, func, reg&0xFC));
    dword = port_read(dev, PCI_PORT_DATA);
    return ((uint8_t *) &dword)[reg & 3];
}

static uint16_t read_conf16(device_t* dev,
                            uint32_t bus,
                            uint32_t devno,
                            uint32_t func,
                            uint32_t reg) {
    return (read_conf8(dev, bus, devno, func, reg+0)<<0)|
           (read_conf8(dev, bus, devno, func, reg+1)<<8);
}

static uint32_t read_conf32(device_t* dev,
                            uint32_t bus,
                            uint32_t devno,
                            uint32_t func,
                            uint32_t reg) {
    return (read_conf8(dev, bus, devno, func, reg+0)<< 0)|
           (read_conf8(dev, bus, devno, func, reg+1)<< 8)|
           (read_conf8(dev, bus, devno, func, reg+2)<<16)|
           (read_conf8(dev, bus, devno, func, reg+3)<<24);
}

/* ================================================================= */
/*                         Bus Enumeration                           */
/* ================================================================= */

uint32_t add_function(device_t *dev, uint8_t bus, uint8_t devno, uint8_t func) {

    device_t *t;
    class_t cls;
    reslist_t reslist = {0, NULL};
    resource_t *resv;
    pci_config_t pci_config;
    int i;

    /* debugging information */
#if 0
    printk("BUS: %d, DEV: %d, FUNC: %d, ", bus, devno, func);
    printk("INTP: %x, ", read_conf8(dev, bus, devno, func, 0x3D));
    printk("INTL: %x, ", read_conf8(dev, bus, devno, func, 0x3C));
    printk("Type: %x\n", read_conf32(dev, bus, devno, func, 0x08));
#endif

    /* class of the PCI device: */
    cls.bus    = BUS_PCI;
    cls.base   = read_conf8(dev, bus, devno, func, PCI_REG_CLASS);
    cls.sub    = read_conf8(dev, bus, devno, func, PCI_REG_SUBCLASS);
    cls.progif = read_conf8(dev, bus, devno, func, PCI_REG_PROG_IF);

    /* resources of the PCI device: */
    resv = (resource_t *) kmalloc(sizeof(resource_t)*7);
    if (resv == NULL)
        return ENOMEM;
    resv[0].type = RESOURCE_TYPE_IRQ;
    resv[0].data.irq.number = read_conf8(dev, bus, devno, func,
                                         PCI_REG_INTERRUPT_LINE);
    for (i = 0; i < 6; i++) {
        uint32_t bar = read_conf32(dev, bus, devno, func,
                                   PCI_REG_BASE_ADDRESS_0 + i*4);
        if (bar & 1) {
            /* I/O */
            resv[i+1].type = RESOURCE_TYPE_PORT;
            resv[i+1].data.mem.base = bar & 0xFFFFFFF0;
        } else {
            /* Memory */
            resv[i+1].type = RESOURCE_TYPE_MEM;
            resv[i+1].data.port.base = bar & 0xFFFFFFF0;
        }
    }

    /* resource list: */
    reslist.count = 7;
    reslist.list  = resv;

    /* config structure: */
    pci_config.vendor_id = read_conf16(dev,bus,devno,func,PCI_REG_VENDOR_ID);
    pci_config.device_id = read_conf16(dev,bus,devno,func,PCI_REG_DEVICE_ID);
    pci_config.bus       = bus;
    pci_config.devno     = devno;
    pci_config.func      = func;
    pci_config.master    = dev;

    /* now add pic1: */
    dev_add(&t, dev->child_bus, cls, reslist, &pci_config);

}

void scan_bus(device_t *dev, uint8_t bus) {

    int32_t i, j, mul;
    uint16_t vendor;
    uint8_t subbus;

    /* loop on devices */
    for (i = 0; i < 32; i++) {
        if (read_conf16(dev, bus, i, 0, PCI_REG_VENDOR_ID) != 0xFFFF) {
            /* device exists */
            j = 0;
            do {
                /* check if multifunction? */
                if (!j) {
                    mul = read_conf8(dev, bus, i, 0, PCI_REG_HEADER_TYPE)>>7;
                } else {
                    if (read_conf16(dev, bus, i, j, PCI_REG_VENDOR_ID)==0xFFFF)
                        continue;
                }
                /* check if bus bridge */
                if (read_conf8(dev, bus, i, j, PCI_REG_CLASS) == 0x06) {
                    /* check if PCI to PCI bridge */
                    if (read_conf8(dev, bus, i, j, PCI_REG_SUBCLASS) == 0x04) {
                        /* read secondary bus */
                        subbus = read_conf8(dev, bus, i, j, 0x19);
                        scan_bus(dev, subbus);
                    }
                }
                add_function(dev, bus, i, j);
            } while (mul && ++j < 8);
        }
    }

}

/* ================================================================= */
/*                            Interface                              */
/* ================================================================= */


uint32_t pci_probe(device_t *dev, void *config) {

    bus_t *pcibus;

    /* splash */
    printk("Initializing PCI bus driver...\n");

    /* create PCI bus */
    dev_mkbus(&pcibus, BUS_PCI, dev);

    /* enumerate PCI buses */
    scan_bus(dev, 0);

    /* done */
    return ESUCCESS;

}

uint32_t pci_read(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    return ESUCCESS;
}

uint32_t pci_write(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    return ESUCCESS;
}

uint32_t pci_ioctl(device_t* dev, uint32_t c, void *data) {
    return ESUCCESS;
}

uint32_t pci_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
