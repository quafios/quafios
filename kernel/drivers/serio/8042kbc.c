/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Intel 8042 Chip Device Driver.                   | |
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
uint32_t i8042_probe(device_t *, void *);
uint32_t i8042_read (device_t *, uint64_t, uint32_t, char *);
uint32_t i8042_write(device_t *, uint64_t, uint32_t, char *);
uint32_t i8042_ioctl(device_t *, uint32_t, void *);
uint32_t i8042_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_ISA, BASE_ISA_INTEL, SUB_ISA_INTEL_8042, IF_ANY}
};

/* driver_t structure that identifies this driver: */
driver_t i8042_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "i8042_serio",
    /* probe:     */ i8042_probe,
    /* read:      */ i8042_read,
    /* write:     */ i8042_write,
    /* ioctl:     */ i8042_ioctl,
    /* irq:       */ i8042_irq
};

typedef struct {
    uint32_t  iotype;
    uint32_t  iodata;
    uint32_t  iocmd;
    uint32_t  firmware;
    device_t* dev;
} info_t;

/* i8042 Status Register:  */
/* ----------------------- */
#define STATUS_OBF      1       /* Output buffer full? */
#define STATUS_IBF      2       /* Input  buffer full? */
#define STATUS_SF       4       /* System Flag.        */
#define STATUS_CD       8       /* 0: Data written to Data port.
                                 * 1: Data written to Command port.
                                 */

/* =================================================================== */
/*                         i8042 Basic Routines                        */
/* =================================================================== */

static uint8_t status(info_t *info) {
    /* The command port, when read, gives the status register. */
    return ioread(1, info->iotype, info->iocmd, 0);
}

static void command(info_t *info, unsigned char cmd) {
    /* send a command to the i8042: */
    while(status(info) & STATUS_IBF); /* must wait till buffer clears. */
    iowrite(1, info->iotype, cmd, info->iocmd, 0);
}

static uint16_t data_read(info_t *info) {
    /* read data from the output buffer of i8042: */
    uint64_t timeout = ticks + 10; /* 0.1 second. */

    /* wait till is full */
    while(!(status(info) & STATUS_OBF))
        /* timeout? */
        if (ticks > timeout) {
                return 0xFFFF;
        }

    return ioread(1, info->iotype, info->iodata, 0);
}

static void data_write(info_t *info, unsigned char data) {
    /* write data to the data input buffer of i8042: */
    while(status(info) & STATUS_IBF); /* must wait till buffer clears. */
    iowrite(1, info->iotype, data, info->iodata, 0);
}

/* =================================================================== */
/*               AT Keyboard Controller Firmware Support               */
/* =================================================================== */

/* Controller Commands: */
#define READ_RAM_BYTE00         0x20  /* Read byte 0 of controller's RAM */
#define READ_RAM_BYTE01         0x21
#define READ_RAM_BYTE02         0x22
#define READ_RAM_BYTE03         0x23
#define READ_RAM_BYTE04         0x24
#define READ_RAM_BYTE05         0x25
#define READ_RAM_BYTE06         0x26
#define READ_RAM_BYTE07         0x27
#define READ_RAM_BYTE08         0x28
#define READ_RAM_BYTE09         0x29
#define READ_RAM_BYTE0A         0x2A
#define READ_RAM_BYTE0B         0x2B
#define READ_RAM_BYTE0C         0x2C
#define READ_RAM_BYTE0D         0x2D
#define READ_RAM_BYTE0E         0x2E
#define READ_RAM_BYTE0F         0x2F
#define READ_RAM_BYTE10         0x30
#define READ_RAM_BYTE11         0x31
#define READ_RAM_BYTE12         0x32
#define READ_RAM_BYTE13         0x33
#define READ_RAM_BYTE14         0x34
#define READ_RAM_BYTE15         0x35
#define READ_RAM_BYTE16         0x36
#define READ_RAM_BYTE17         0x37
#define READ_RAM_BYTE18         0x38
#define READ_RAM_BYTE19         0x39
#define READ_RAM_BYTE1A         0x3A
#define READ_RAM_BYTE1B         0x3B
#define READ_RAM_BYTE1C         0x3C
#define READ_RAM_BYTE1D         0x3D
#define READ_RAM_BYTE1E         0x3E
#define READ_RAM_BYTE1F         0x3F
#define WRITE_RAM_BYTE00        0x60  /* Write byte 0 of controller's RAM */
#define WRITE_RAM_BYTE01        0x61
#define WRITE_RAM_BYTE02        0x62
#define WRITE_RAM_BYTE03        0x63
#define WRITE_RAM_BYTE04        0x64
#define WRITE_RAM_BYTE05        0x65
#define WRITE_RAM_BYTE06        0x66
#define WRITE_RAM_BYTE07        0x67
#define WRITE_RAM_BYTE08        0x68
#define WRITE_RAM_BYTE09        0x69
#define WRITE_RAM_BYTE0A        0x6A
#define WRITE_RAM_BYTE0B        0x6B
#define WRITE_RAM_BYTE0C        0x6C
#define WRITE_RAM_BYTE0D        0x6D
#define WRITE_RAM_BYTE0E        0x6E
#define WRITE_RAM_BYTE0F        0x6F
#define WRITE_RAM_BYTE10        0x70
#define WRITE_RAM_BYTE11        0x71
#define WRITE_RAM_BYTE12        0x72
#define WRITE_RAM_BYTE13        0x73
#define WRITE_RAM_BYTE14        0x74
#define WRITE_RAM_BYTE15        0x75
#define WRITE_RAM_BYTE16        0x76
#define WRITE_RAM_BYTE17        0x77
#define WRITE_RAM_BYTE18        0x78
#define WRITE_RAM_BYTE19        0x79
#define WRITE_RAM_BYTE1A        0x7A
#define WRITE_RAM_BYTE1B        0x7B
#define WRITE_RAM_BYTE1C        0x7C
#define WRITE_RAM_BYTE1D        0x7D
#define WRITE_RAM_BYTE1E        0x7E
#define WRITE_RAM_BYTE1F        0x7F
#define TEST_PASSWORD_INSTALLED 0xA4
#define LOAD_PASSWORD_INSTALLED 0xA5
#define ENABLE_PASSWORD         0xA6
#define DISABLE_PS2_PORT2       0xA7
#define ENABLE_PS2_PORT2        0xA8
#define TEST_PS2_PORT2          0xA9
#define TEST_PS2_CONTROLLER     0xAA
#define TEST_PS2_PORT1          0xAB
#define DIAGNOSTIC_DUMP         0xAC
#define DISABLE_PS2_PORT1       0xAD
#define ENABLE_PS2_PORT1        0xAE
#define READ_CTRL_INPUT_PORT    0xC0
#define COPY_0_3_INPUT_STATUS   0xC1 /* copy input[0..3] to status[4..7] */
#define COPY_4_7_INPUT_STATUS   0xC2 /* copy input[4..7] to status[4..7] */
#define READ_CTRL_OUTPUT        0xD0 /* read controller output port */
#define WRITE_CTRL_OUTPUT       0xD1 /* write next byte to ctlr output port */
#define WRITE_NEXT_BYTE_PS2OP1  0xD2 /* to first PS/2 port output buffer */
#define WRITE_NEXT_BYTE_PS2OP2  0xD3 /* to second PS/2 port output buffer */
#define WRITE_NEXT_BYTE_PS2IP2  0xD4 /* to second PS/2 port input buffer */
#define READ_TEST_INPUTS        0xE0
#define PULSE_OUTPUT_LINE       0xF0

#define PULSE_LINE              0
#define DONT_PULSE              1
#define RESET_COMMAND           (PULSE_OUTPUT_LINE+\
                                 (PULSE_LINE<<0)+ /* Reset Line */\
                                 (DONT_PULSE<<1)+\
                                 (DONT_PULSE<<2)+\
                                 (DONT_PULSE<<3))

/* Configuration byte (RAM byte 0):
 * ====================================
 * Bit 0: First  PS/2 port Interrupt (0: disable, 1: enable).
 * Bit 1: Second PS/2 port Interrupt (0: disable, 1: enable).
 * Bit 2: System Flag, System passed POST? (1: passed, 0: can never happen).
 * Bit 3: Should be zero.
 * Bit 4: First  PS/2 port Clock (0: enabled, 1: disabled).
 * Bit 5: Second PS/2 port Clock (0: enabled, 1: disabled).
 * Bit 6: First PS/2 port Translation (0: disabled, 1: enabled).
 * Bit 7: Must be zero.
 */

/* Controller Output Port (Command 0xD0):
 * ======================================
 * Bit 0: SYSTEM RESET, always 1. setting it to 0 = "Reset forever".
 * Bit 1: A20 gate.
 * Bit 2: Second PS/2 port clock.
 * Bit 3: Second PS/2 port data.
 * Bit 4: First  PS/2 output buffer full (connected to IRQ1)
 * Bit 5: Second PS/2 output buffer full (connected to IRQC)
 * Bit 6: First PS/2 port clock.
 * Bit 7: First PS/2 port data.
 */

static uint32_t atkbc_init(info_t *info) {
    /* initialize the small piece of software loaded
     * on the fantastic i8042 chip.
     */

    resource_t  *resv;
    reslist_t   reslist;
    class_t     cls;
    device_t    *t;
    uint32_t    channel0_supported = 1;
    uint32_t    channel1_supported = 1;
    uint8_t     conf;

    /* again, i love the user when he looks like "HerpDerp" while
     * these messages are popping up his screen.
     */
    printk("i8042 firmware type: IBM PS/2 Keyboard controller.\n");

    /* Disable First & Second PS/2 devices: */
    command(info, DISABLE_PS2_PORT1);
    command(info, DISABLE_PS2_PORT2);

    /* some data might be waiting at the data port, read it. */
    while(data_read(info) != 0xFFFF);

    /* Set the configuration byte: */
    command(info, READ_RAM_BYTE00);
    conf = data_read(info);
    conf &= 0xBC; /* disable interrupts and bit 6 (translation). */
    command(info, WRITE_RAM_BYTE00);
    data_write(info, conf);
    /* making sure bit 5 is set: */
    if (!(conf & 0x20)) {
        /* something wrong with channel 1. it is not supported. */
        channel1_supported = 0;
        printk("PS/2 Second channel is not supported\n");
    }

    /* Controller Self Test: */
    command(info, TEST_PS2_CONTROLLER);
    if (data_read(info) != 0x55)
        return ENODEV; /* fatal hardware error. */

    /* check again for channel 1 existence: */
    if (channel1_supported) {
        /* enable & disable. */
        command(info, ENABLE_PS2_PORT2);
        command(info, READ_RAM_BYTE00);
        conf = data_read(info);
        command(info, DISABLE_PS2_PORT2);
        if (conf & 0x20) {
            /* is still set. */
            channel1_supported = 0;
            printk("PS/2 Second channel is not supported\n");
        }
    }

    /* Test First PS/2 port: */
    command(info, TEST_PS2_PORT1);
    if (data_read(info) != 0x00) {
        channel0_supported = 0;
        printk("First PS/2 port is buggy/not found\n");
    }

    /* Test Second PS/2 port: */
    if (channel1_supported) {
        command(info, TEST_PS2_PORT2);
        if (data_read(info) != 0x00) {
            channel1_supported = 0;
            printk("Second PS/2 port is buggy/not found\n");
        }
    }

    /* Enable First PS/2 Device: */
    if (channel0_supported) {
        command(info, ENABLE_PS2_PORT1);
        /* Set the configuration byte: */
        command(info, READ_RAM_BYTE00);
        conf = data_read(info);
        conf |= 0x41; /* enable interrupt and bit 6 (translation). */
        command(info, WRITE_RAM_BYTE00);
        data_write(info, conf);
    }

    /* Enable Second PS/2 Device: */
    if (channel1_supported) {
        /* Set the configuration byte: */
        command(info, READ_RAM_BYTE00);
        conf = data_read(info);
        conf |= 0x02; /* enable interrupt. */
        command(info, WRITE_RAM_BYTE00);
        data_write(info, conf);
        /* Enable the mouse port */
        command(info, ENABLE_PS2_PORT2);
    }

    if (channel0_supported) {
        /* Load the PS/2 keyboard driver: */
        cls.bus    = BUS_SERIO;
        cls.base   = BASE_i8042_ATKBC;
        cls.sub    = SUB_i8042_ATKBC_PS2KEYBOARD;
        cls.progif = IF_ANY;
        resv = (resource_t *) kmalloc(sizeof(resource_t)*1);
        if (resv == NULL)
            return ENOMEM;
        resv[0].type = RESOURCE_TYPE_IRQ;
        resv[0].data.irq.number = info->dev->resources.list[2].data.irq.number;
        reslist.count = 1;
        reslist.list  = resv;
        dev_add(&t, (bus_t *) info->dev->child_bus, cls, reslist, NULL);
    }

    if (channel1_supported) {
        /* Load the PS/2 Mouse driver: */
        cls.bus    = BUS_SERIO;
        cls.base   = BASE_i8042_ATKBC;
        cls.sub    = SUB_i8042_ATKBC_PS2MOUSE;
        cls.progif = IF_ANY;
        resv = (resource_t *) kmalloc(sizeof(resource_t)*1);
        if (resv == NULL) return ENOMEM;
        resv[0].type = RESOURCE_TYPE_IRQ;
        resv[0].data.irq.number = info->dev->resources.list[3].data.irq.number;
        reslist.count = 1;
        reslist.list  = resv;
        dev_add(&t, (bus_t *) info->dev->child_bus, cls, reslist, NULL);
    }

    /* Reallocate resv of the controller: */
    resv = (resource_t *) kmalloc(sizeof(resource_t)*2);
    resv[0] = info->dev->resources.list[0];
    resv[1] = info->dev->resources.list[1];
    info->dev->resources.count = 2;
    kfree(info->dev->resources.list);
    info->dev->resources.list = resv;

    return ESUCCESS;
}

static uint32_t atkbc_sendrec(info_t *info, atkbc_sendrec_t *data) {
    /* Atomic Send/Receive: */
    int32_t i;
    for (i = 0; i < data->send; i++) {
        /* send interface command command: */
        if (data->channel == 1)
            /* second channel: */
            command(info, WRITE_NEXT_BYTE_PS2IP2);
        data_write(info, data->sbuff[i]);
    }
    for (i = 0; i < data->receive; i++)
        data->rbuff[i] = data_read(info);
    return ESUCCESS;
}

static uint32_t atkbc(info_t *info, uint32_t cmd, void *data) {
    switch (cmd) {
        case i8042_ATKBC_INIT:
            return atkbc_init(info);
        case i8042_ATKBC_SENDREC:
            return atkbc_sendrec(info, data);
        default:
            return EBUSY; /* invalid command. */
    }
}

/* =================================================================== */
/*                        i8042 Basic Interface                        */
/* =================================================================== */

uint32_t i8042_probe(device_t *dev, void *config) {
    /* some variables */
    bus_t *seriobus;
    uint32_t err;
    i8042_init_t *cfg = config;

    /* Allocate info_t structure: */
    info_t *info = (info_t *) kmalloc(sizeof(info_t));
    dev->drvreg = (uint32_t) info;
    if (info == NULL)
            return ENOMEM;
    info->dev = dev;

    /* let the user feel that Quafios is an advanced OS... */
    printk("i8042 peripheral controller device driver.\n");

    /* Get I/O information: */
    info->iotype = dev->resources.list[0].type;
    info->iodata = dev->resources.list[0].data.port.base;
    info->iocmd  = dev->resources.list[1].data.port.base;

    /* Create the serio bus: */
    if (err = dev_mkbus(&seriobus, BUS_SERIO, dev))
        return err;

    /* Load firmware: */
    info->firmware = cfg->firmware_code;
    return i8042_ioctl(dev, 0 /* initialize */, NULL);
}

uint32_t i8042_read(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    return ESUCCESS;
}

uint32_t i8042_write(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    return ESUCCESS;
}

uint32_t i8042_ioctl(device_t *dev, uint32_t cmd, void *data) {
    /* send a firmware specific command. */

    /* Get info_t structure: */
    info_t *info = (info_t *) dev->drvreg;
    if (info == NULL)
        return ENOMEM;

    switch (info->firmware) {
        case i8042_ATKBC:
            return atkbc(info, cmd, data);
        default:
            return ENOENT;
    }
}

uint32_t i8042_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
