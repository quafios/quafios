/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Intel 8259A PIC Device Driver                    | |
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
#include <pic/8259A.h>

/* Prototypes: */
uint32_t i8259_probe(device_t *, void *);
uint32_t i8259_read (device_t *, uint64_t, uint32_t, char *);
uint32_t i8259_write(device_t *, uint64_t, uint32_t, char *);
uint32_t i8259_ioctl(device_t *, uint32_t, void *);
uint32_t i8259_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_ISA, BASE_ISA_INTEL, SUB_ISA_INTEL_8259A, IF_ANY}
};

/* driver_t structure that identifies this driver: */
driver_t i8259_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "8259A_pic",
    /* probe:     */ i8259_probe,
    /* read:      */ i8259_read,
    /* write:     */ i8259_write,
    /* ioctl:     */ i8259_ioctl,
    /* irq:       */ i8259_irq
};

typedef struct {
    uint32_t iobase;  /* IO Addresses Base.  */
    uint32_t iotype;  /* Memory I/O or Port? */
    uint32_t baseirq; /* IRQ Base.           */
    uint32_t cascade; /* described in pic.h  */
    uint32_t casirq;  /* Cascade IRQ.        */
    device_t *master; /* the master PIC      */
} info_t;

/* ================================================================= */
/*                             Chip I/O                              */
/* ================================================================= */

#define PIC8259A_CMD            0 /* Command Port */
#define PIC8259A_DATA           1 /* Data Port    */

static void cmd(device_t* dev, uint8_t c) {
    /* write command */
    info_t *info = (info_t *) dev->drvreg;
    iowrite(1, info->iotype, c, info->iobase, PIC8259A_CMD);
}

static void data(device_t* dev, uint8_t d) {
    /* write data */
    info_t *info = (info_t *) dev->drvreg;
    iowrite(1, info->iotype, d, info->iobase, PIC8259A_DATA);
}

/* ================================================================= */
/*                            Interface                              */
/* ================================================================= */

uint32_t i8259_probe(device_t *dev, void *config) {

    /* local variables */
    i8259_init_t* cfg = config;
    uint8_t ICW1, ICW2, ICW3, ICW4, OCW1;
    int32_t i;

    /* create info_t structure: */
    info_t *info = (info_t *) kmalloc(sizeof(info_t));
    dev->drvreg = (uint32_t) info;
    if (info == NULL)
        return ENOMEM; /* i am sorry :D */

    /* Store data: */
    info->iobase  = dev->resources.list[0].data.port.base;
    info->iotype  = dev->resources.list[0].type;
    info->baseirq = cfg->baseirq;
    info->cascade = cfg->cascade;
    info->casirq  = cfg->casirq;
    info->master  = cfg->master;
#if 0
    printk("iobase: %x, baseirq: %d, cascade: %d, casirq: %d\n",
           info->iobase, info->baseirq, info->cascade, info->casirq);
#endif

    /* initialize 8259A chip: */

    /* Initialization Command Word 1 [Command Port]:
     * bit0: 0: no ICW4 needed, 1: ICW4 needed
     * bit1: 0: cascading, 1: single.
     * bit2: 0: 8-byte int vectors, 1: 4-byte int vectors.
     * bit3: 0: edge-triggered, 1: level-triggered.
     * bit4: must be 1 (shows that this is ICW1).
     * bit5-7: should be zeros.
     */
    ICW1 = 0x11 | (info->cascade > 0 ? 0 : 2) | 8;
    cmd(dev, ICW1);

    /* Initialization Command Word 2 [Data Port]:
     * bit0-2: zeros for 80x86 systems.
     * bit3-7: bits3-7 of the vector.
     */
    ICW2 = irq_to_vector(info->baseirq);
    data(dev, ICW2);

    /* Initialization Command Word 3 [Data Port]:
     * for master device:
     * bit n: 0: no slave at IRQ bin n, 1: a slave device is attached.
     * for slave:
     * bits0-2: Master IRQ to which this slave chip is attached.
     * bits3-7: should be zeros.
     */
    if (info->cascade) {
        ICW3 = 0;
        if (info->cascade == CASCADE_MASTER)
            ICW3 = 1 << info->casirq;
        else
            ICW3 = info->casirq;
        data(dev, ICW3);
    }

    /* Initialization Command Word 4 [Data Port]:
     * bit0: 0: MCS 80/85 mode, 1: 80x86 mode
     * bit1: 0: Normal EOI, 1: Auto EOI.
     * bit2-3: 00, 01: not buffered, 10: buffered mode (slave), 11: master.
     * bit4: 0: Sequential, 1: Special Fully Nested Mode.
     * bit5-7: should be zero.
     */
    ICW4 = 0x01;
    data(dev, ICW4);

    /* Operation Control Word 1 {interrupt mask register} [Data Port]:
     * bit n: 0: service IRQ n, 1: mask off.
     */
    OCW1 = 0;
    data(dev, OCW1);

    /* Initialize IRQs: */
    for (i = 0; i < 8; i++)
        irq_setup(info->baseirq + i, dev);

    /* Enable IRQ system: */
    if (cfg->irqenable)
        enable_irq_system();

    /* return: */
    return ESUCCESS;
}

uint32_t i8259_read(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    return ESUCCESS;
}

uint32_t i8259_write(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    return ESUCCESS;
}

uint32_t i8259_ioctl(device_t* dev, uint32_t c, void *data) {
    /* local variables */
    uint8_t OCW2;

    /* get info_t structure: */
    info_t *info = (info_t *) dev->drvreg;
    if (info == NULL)
            return ENOMEM; /* i am sorry :D */

    /* send EOI (End of interrupt) command:
     * Operation Control Word 2 {interrupt command register}
     *                                              [Command Port]:
     * bits 0-2: IRQ level to act upon.
     * bits 3-4: must be zero for OCW2.
     * bits 5-7: EOI Code: 001: non-specific EOI command.
     *                     011: specific EOI command.
     *                     100: rotate in automatic EOI mode.
     *                     101: rotate on non-specific EOI command.
     *                     111: rotate on specific EOI command.
     *                     010: NOP.
     *                     110: set priority command (using bits 0-2).
     */
    OCW2 = 0x20;
    cmd(dev, OCW2);

    if (info->cascade == CASCADE_SLAVE) {
        /* Send EOI to the master */
        dev_ioctl(info->master, PIC_CMD_EOI, NULL);
    }

    return ESUCCESS;
}

uint32_t i8259_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
