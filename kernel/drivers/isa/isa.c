/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> ISA Bus Device Driver                            | |
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
#include <sys/scheduler.h>

/* the hardware attached to ISA bus */
#include <timer/8253.h>
#include <pic/8259A.h>
#include <serio/8042kbc.h>
#include <video/generic.h>

/* Prototypes: */
uint32_t isa_probe(device_t *, void *);
uint32_t isa_read (device_t *, uint64_t, uint32_t, char *);
uint32_t isa_write(device_t *, uint64_t, uint32_t, char *);
uint32_t isa_ioctl(device_t *, uint32_t, void *);
uint32_t isa_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_HOST, BASE_HOST_SUBSYSTEM, SUB_HOST_SUBSYSTEM_ISA, IF_ANY}
};

/* driver_t structure that identifies this driver: */
driver_t isa_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "isa",
    /* probe:     */ isa_probe,
    /* read:      */ isa_read,
    /* write:     */ isa_write,
    /* ioctl:     */ isa_ioctl,
    /* irq:       */ isa_irq
};

/* ================================================================= */
/*                            Interface                              */
/* ================================================================= */

uint32_t isa_probe(device_t *dev, void *config) {

    bus_t *isabus;
    bus_t *hostbus;
    device_t *t;
    class_t cls;
    reslist_t reslist;
    resource_t *resv; /* vector of resources. */
    i8259_init_t pic1_config, pic2_config;
    i8253_init_t timer_config;
    i8042_init_t i8042_config;
    uint32_t err;

    /* Create the isa bus, this allows legacy support for
     * all the old standard system bus of IBM PC, and for
     * most devices appear on it.
     */
    err = dev_mkbus(&isabus, BUS_ISA, NULL);
    if (err) return err; /* error while creating the bus :'( */

    /* Add Legacy Devices of IBM PC :D
     * That's really fantastically amazingly interesting :)
     */

    /* I) Intel's 8259A PIC:
     * ----------------------
     * The default interrupt controller used in the original
     * IBM personal computer.
     * In IBM PC/AT, they were actually two PICs, one was master
     * and the other was slave.
     */
    /* class of the PIC device: */
    cls.bus    = BUS_ISA;
    cls.base   = BASE_ISA_INTEL;
    cls.sub    = SUB_ISA_INTEL_8259A;
    cls.progif = IF_ANY;
    /* resources of the master pic: */
    resv = (resource_t *) kmalloc(sizeof(resource_t)*1);
    if (resv == NULL) return ENOMEM;
    resv[0].type = RESOURCE_TYPE_PORT;
    resv[0].data.port.base = 0x20; /* standard. */
    resv[0].data.port.size = 2;    /* 2 bytes. */
    /* resource list: */
    reslist.count = 1;
    reslist.list  = resv;
    /* config structure: */
    pic1_config.baseirq   = 0;
    pic1_config.cascade   = CASCADE_MASTER;
    pic1_config.casirq    = 2;
    pic1_config.master    = NULL;
    pic1_config.irqenable = 0;
    /* now add pic1: */
    dev_add(&t, isabus, cls, reslist, &pic1_config);

    /* resources of secondary pic: */
    resv = (resource_t *) kmalloc(sizeof(resource_t)*1);
    if (resv == NULL) return ENOMEM;
    resv[0].type = RESOURCE_TYPE_PORT;
    resv[0].data.port.base = 0xA0; /* standard. */
    resv[0].data.port.size = 2;    /* 2 bytes.  */
    /* resource list: */
    reslist.count = 1;
    reslist.list  = resv;
    /* config: */
    pic2_config.baseirq   = 8;
    pic2_config.cascade   = CASCADE_SLAVE;
    pic2_config.casirq    = 2;
    pic2_config.master    = t;
    pic2_config.irqenable = 1;
    /* now add pic2: */
    dev_add(&t, isabus, cls, reslist, &pic2_config);

    /* II) Intel's 8253 Timer:
     * ------------------------
     * The default programmable interval timer used in the original
     * IBM personal computer.
     * We are gonna use it to execute the scheduler each specific
     * interval of time.
     */
    /* class for the 8253 chip: */
    cls.bus    = BUS_ISA;
    cls.base   = BASE_ISA_INTEL;
    cls.sub    = SUB_ISA_INTEL_8253;
    cls.progif = IF_ANY;
    /* resources: */
    resv = (resource_t *) kmalloc(sizeof(resource_t)*2);
    if (resv == NULL) return ENOMEM;
    resv[0].type = RESOURCE_TYPE_PORT;
    resv[0].data.port.base = 0x40; /* standard. */
    resv[0].data.port.size = 4;    /* 4 bytes. */
    resv[1].type = RESOURCE_TYPE_IRQ;
    resv[1].data.irq.number = 0;   /* 8253 is connected to IRQ0 line. */
    /* resource list: */
    reslist.count = 2;
    reslist.list  = resv;
    /* config:
     * i8253 contains 3 clocks, they are used in IBM PC as follows:
     * CLK 0 -> connected to IRQ0.
     * CLK 1 -> almost not used
     * CLK 2 -> connected to PC speaker.
     * The oscillator of i8253 runs at a frequency of
     * 1.193181666666 MHz (666666 is recurring) in IBM PC.
     * We will make use of it to generate IRQ0 each 10ms, so that
     * Quafios task scheduler is called every 10ms.
     * According to physics:
     * T = 1 / f; < periodic time >
     * counter = required_interval / T
     * therefore: counter = required_interval * f
     *                    = 0.01 * 1193181 = 11931
     */
    timer_config.clock[0].count = 1193181 * SCHEDULER_INTERVAL;
    timer_config.clock[0].mode  = 3;     /* CLK 0 should operate in mode 3. */
    timer_config.clock[0].catch_irq = 1; /* catch. */

    timer_config.clock[1].count = 0;     /* ignore it. */
    timer_config.clock[1].mode  = 0;     /* ignored. */
    timer_config.clock[1].catch_irq = 0; /* no irq. */

    timer_config.clock[2].count = 0;     /* ignore it. */
    timer_config.clock[2].mode  = 0;     /* ignored. */
    timer_config.clock[2].catch_irq = 0; /* ignored. */

    /* now add the timer: */
    dev_add(&t, isabus, cls, reslist, &timer_config);

    /* configure the scheduler: */
    scheduler_irq = 0; /* IRQ0. */

    /* III) Intel's 8042 Peripheral Controller:
     * -----------------------------------------
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

    /* IV) Video Graphics Array (VGA)
     * -------------------------------
     * VGA is a piece of hardware that's responsible for
     * adapting graphics on screen.
     */
    cls.bus    = BUS_ISA;
    cls.base   = BASE_ISA_IBM;
    cls.sub    = SUB_ISA_IBM_VGA;
    cls.progif = IF_ANY;
    /* resources: */
    resv = (resource_t *) kmalloc(sizeof(resource_t)*4);
    if (resv == NULL) return ENOMEM;
    resv[0].type = RESOURCE_TYPE_PORT;
    resv[0].data.port.base = 0x0;
    resv[0].data.port.size = 1;
    /* resource list: */
    reslist.count = 4;
    reslist.list  = resv;
    /* now add the VGA: */
    dev_add(&t, isabus, cls, reslist, NULL);

    /* finally... */
    return ESUCCESS;
}

uint32_t isa_read(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    return ESUCCESS;
}

uint32_t isa_write(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    return ESUCCESS;
}

uint32_t isa_ioctl(device_t* dev, uint32_t c, void *data) {
    return ESUCCESS;
}

uint32_t isa_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
