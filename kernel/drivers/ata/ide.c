/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> ATA/ATAPI IDE Device Driver.                     | |
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
#include <sys/bootinfo.h>
#include <sys/scheduler.h>
#include <pci/pci.h>
#include <ata/ide.h>

/* Prototypes: */
uint32_t ide_probe(device_t *, void *);
uint32_t ide_read (device_t *, uint64_t, uint32_t, char *);
uint32_t ide_write(device_t *, uint64_t, uint32_t, char *);
uint32_t ide_ioctl(device_t *, uint32_t, void *);
uint32_t ide_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_PCI, BASE_PCI_STORAGE, SUB_PCI_STORAGE_IDE, IF_ANY}
};

/* driver_t structure that identifies this driver: */
driver_t ide_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "ide",
    /* probe:     */ ide_probe,
    /* read:      */ ide_read,
    /* write:     */ ide_write,
    /* ioctl:     */ ide_ioctl,
    /* irq:       */ ide_irq
};

typedef struct {
    uint32_t     command_reg_base; /* command block registers */
    uint32_t     command_reg_type; /* memory-mapped or I/O? */
    uint32_t     control_reg_base; /* control block registers */
    uint32_t     control_reg_type; /* memory-mapped or I/O? */
    uint32_t     pci_dma_reg_base; /* pci dma registers */
    uint32_t     pci_dma_reg_type; /* memory-mapped or I/O? */
    uint32_t     irqnum;           /* IRQ */
    ata_drive_t  drives[2];        /* information about the drives */
} channel_t;

typedef struct {
    /* device pointer */
    device_t *dev;
    /* pci_info */
    uint8_t bus;
    uint8_t devno;
    uint8_t func;
    device_t *master;
    /* ide info */
    channel_t channels[2];
} info_t;

/* ================================================================= */
/*                            Register I/O                           */
/* ================================================================= */

/* Addressing Modes:
 * -> CHS
 * -> LBA28
 * -> LBA48
 * Command protocols/types:
 * -> nodata
 * -> pio
 * -> dma
 * -> packet
 * Polling methods:
 * -> polling status regs
 * -> IRQs
 */

/* register indices */
#define ATA_REG_DATA            0 /* R/W data port (for PIO, not DMA) */
#define ATA_REG_FEATURES_ERRORS 1 /* write features/read error */
#define ATA_REG_SECTOR_COUNT    2 /* sector count */
#define ATA_REG_SECTOR          3 /* sector */
#define ATA_REG_CYLINDER_LOW    4 /* cylinder low */
#define ATA_REG_CYLINDER_HIGH   5 /* cylinder high */
#define ATA_REG_HEAD_DRIVE_SEL  6 /* head and drive select */
#define ATA_REG_COMMAND_STATUS  7 /* write command/read status */

#define ATA_REG_CTRL_ALT_STATUS 8 /* control reg/alternate status */

static uint8_t read_reg(info_t *info,
                        uint32_t channel,
                        uint32_t reg) {
    uint32_t type, base, offset;
    if (reg != ATA_REG_CTRL_ALT_STATUS) {
        base   = info->channels[channel].command_reg_base;
        type   = info->channels[channel].command_reg_type;
        offset = reg;
    } else {
        base   = info->channels[channel].control_reg_base;
        type   = info->channels[channel].control_reg_type;
        offset = 0;
    }
    /*printk("READ @%x\n", base+offset);*/
    return ioread(1, type, base, offset);
}

static uint16_t read_reg16(info_t *info,
                           uint32_t channel,
                           uint32_t reg) {
    uint32_t type, base, offset;
    if (reg != ATA_REG_CTRL_ALT_STATUS) {
        base   = info->channels[channel].command_reg_base;
        type   = info->channels[channel].command_reg_type;
        offset = reg;
    } else {
        base   = info->channels[channel].control_reg_base;
        type   = info->channels[channel].control_reg_type;
        offset = 0;
    }
    /*printk("READ @%x\n", base+offset);*/
    return ioread(2, type, base, offset);
}

static void write_reg(info_t *info,
                      uint32_t channel,
                      uint32_t reg,
                      uint8_t val) {
    uint32_t type, base, offset;
    if (reg != ATA_REG_CTRL_ALT_STATUS) {
        base   = info->channels[channel].command_reg_base;
        type   = info->channels[channel].command_reg_type;
        offset = reg;
    } else {
        base   = info->channels[channel].control_reg_base;
        type   = info->channels[channel].control_reg_type;
        offset = 0;
    }
    /*printk("WRITE @%x:%x\n", base+offset, val);*/
    iowrite(1, type, val, base, offset);
}

static void write_reg16(info_t *info,
                        uint32_t channel,
                        uint32_t reg,
                        uint16_t val) {
    uint32_t type, base, offset;
    if (reg != ATA_REG_CTRL_ALT_STATUS) {
        base   = info->channels[channel].command_reg_base;
        type   = info->channels[channel].command_reg_type;
        offset = reg;
    } else {
        base   = info->channels[channel].control_reg_base;
        type   = info->channels[channel].control_reg_type;
        offset = 0;
    }
    /*printk("WRITE @%x:%x\n", base+offset, val);*/
    iowrite(2, type, val, base, offset);
}

/* ================================================================= */
/*                    Control/Alt-Status Register                    */
/* ================================================================= */

/* layout of device control register */
#define DEVCTRL_MASK_NIEN       0x02 /* $IEN$ -> Not Interrupt Enabled */
#define DEVCTRL_MASK_SRST       0x04 /* software reset all ATA drives on bus */
#define DEVCTRL_MASK_HOB        0x80 /* read back the high order byte of the
                                        last LBA48 value sent to an IO port */

static uint8_t read_status_alt(info_t *info, uint32_t channel) {
    return read_reg(info, channel, ATA_REG_CTRL_ALT_STATUS);
}

static void delay_400ns(info_t *info, int channel) {
    read_status_alt(info, channel);
    read_status_alt(info, channel);
    read_status_alt(info, channel);
    read_status_alt(info, channel);
}

static void soft_reset(info_t *info, int channel) {
    /* soft reset all drives on the bus */
    write_reg(info, channel, ATA_REG_CTRL_ALT_STATUS, DEVCTRL_MASK_SRST);
    /* wait for a while */
    sleep(10);
    /* reset the bus to normal operation */
    write_reg(info, channel, ATA_REG_CTRL_ALT_STATUS, DEVCTRL_MASK_NIEN);
    /* wait for a while */
    sleep(10);
}

static void disable_irq(info_t *info, int channel) {
    /* set nIEN */
    write_reg(info, channel, ATA_REG_CTRL_ALT_STATUS, DEVCTRL_MASK_NIEN);
}

static void enable_irq(info_t *info, int channel) {
    /* clear nIEN */
    write_reg(info, channel, ATA_REG_CTRL_ALT_STATUS, 0);
}

/* ================================================================= */
/*                      Command/Status Register                      */
/* ================================================================= */

static uint8_t read_status(info_t *info, uint32_t channel) {
    return read_reg(info, channel, ATA_REG_COMMAND_STATUS);
}

static void write_cmd(info_t *info, uint32_t channel, uint16_t cmd) {
    write_reg(info, channel, ATA_REG_COMMAND_STATUS, cmd);
}

/* ================================================================= */
/*                          Data Registers                           */
/* ================================================================= */

/* layout of head/drivesel reg */
#define DEVSEL_MASK_HEAD        0x0F /* head, or highest 4 bits of LBA */
#define DEVSEL_MASK_SLAVE       0x10 /* 0=master, 1=slave */
#define DEVSEL_MASK_ONE0        0x20 /* this bit should be 1 */
#define DEVSEL_MASK_LBA         0x40 /* 0=CHS, 1=LBA */
#define DEVSEL_MASK_ONE1        0x80 /* this bit should be one */

static uint16_t read_data(info_t *info, uint32_t channel) {
    uint16_t word = read_reg16(info, channel, ATA_REG_DATA);
    return ((word & 0xFF) << 8) | ((word >> 8) & 0xFF);
}

static uint16_t read_data2(info_t *info, uint32_t channel) {
    return read_reg16(info, channel, ATA_REG_DATA);
}

static uint16_t write_data2(info_t *info, uint32_t channel, uint16_t val) {
    write_reg16(info, channel, ATA_REG_DATA, val);
}

static uint8_t read_errors(info_t *info, uint32_t channel) {
    return read_reg(info, channel, ATA_REG_FEATURES_ERRORS);
}

static uint8_t read_seccount(info_t *info, uint32_t channel) {
    return read_reg(info, channel, ATA_REG_SECTOR_COUNT);
}

static uint8_t read_sector(info_t *info, uint32_t channel) {
    return read_reg(info, channel, ATA_REG_SECTOR);
}

static uint8_t read_cylinder_low(info_t *info, uint32_t channel) {
    return read_reg(info, channel, ATA_REG_CYLINDER_LOW);
}

static uint8_t read_cylinder_high(info_t *info, uint32_t channel) {
    return read_reg(info, channel, ATA_REG_CYLINDER_HIGH);
}

static uint8_t read_head(info_t *info, uint32_t channel) {
    return read_reg(info, channel, ATA_REG_HEAD_DRIVE_SEL) & 0x0F;
}

static void select_drive(info_t *info,
                         uint32_t channel,
                         uint32_t drvnum,
                         uint16_t seccount,
                         uint64_t lba,
                         int32_t  mode) {
    ata_drive_t *drive = &info->channels[channel].drives[drvnum];
    uint32_t drivesel = DEVSEL_MASK_ONE0 | DEVSEL_MASK_ONE1;
    uint32_t cylinder, head, sector;
    /* select master or slave */
    if (drvnum)
        drivesel |= DEVSEL_MASK_SLAVE;
    /* select addressing mode */
    switch (mode) {
        case ATA_AMODE_CHS:
            if (lba == -1) {
                /* not initialized */
                cylinder = 0;
                head     = 0;
                sector   = 0;
            } else {
                /* convert LBA to CHS */
                cylinder = lba/(((uint64_t)drive->heads)*drive->sectors);
                head     = (lba/((uint64_t)drive->sectors))%drive->heads;
                sector   = (lba % drive->sectors)+1;
                printk("Cylinder: %d, head: %d, sector: %d\n",
                       cylinder,head,sector);
            }
            /* drivesel register */
            drivesel |= head & 0x0F;
            write_reg(info, channel, ATA_REG_HEAD_DRIVE_SEL, drivesel);
            delay_400ns(info, channel);
            /* write features */
            write_reg(info, channel, ATA_REG_FEATURES_ERRORS, 0);
            /* write other registers */
            write_reg(info, channel, ATA_REG_SECTOR_COUNT, seccount&0xFF);
            write_reg(info, channel, ATA_REG_SECTOR, sector&0xFF);
            write_reg(info, channel, ATA_REG_CYLINDER_LOW, (cylinder>>0)&0xFF);
            write_reg(info, channel, ATA_REG_CYLINDER_HIGH, (cylinder>>8)&0xFF);
            /* done */
            break;
        case ATA_AMODE_LBA28:
            /* drivesel register */
            drivesel |= ((lba>>24) & 0x0F) | DEVSEL_MASK_LBA;
            write_reg(info, channel, ATA_REG_HEAD_DRIVE_SEL, drivesel);
            delay_400ns(info, channel);
            /* write features */
            write_reg(info, channel, ATA_REG_FEATURES_ERRORS, 0);
            /* write other registers */
            write_reg(info, channel, ATA_REG_SECTOR_COUNT, seccount&0xFF);
            write_reg(info, channel, ATA_REG_SECTOR, (lba>>0)&0xFF);
            write_reg(info, channel, ATA_REG_CYLINDER_LOW, (lba>>8)&0xFF);
            write_reg(info, channel, ATA_REG_CYLINDER_HIGH, (lba>>16)&0xFF);
            /* done */
            break;
        case ATA_AMODE_LBA48:
            /* drivesel register */
            drivesel |= DEVSEL_MASK_LBA;
            write_reg(info, channel, ATA_REG_HEAD_DRIVE_SEL, drivesel);
            delay_400ns(info, channel);
            /* write features */
            write_reg(info, channel, ATA_REG_FEATURES_ERRORS, 0);
            /* higher order bytes */
            write_reg(info, channel, ATA_REG_SECTOR_COUNT, (seccount>>8)&0xFF);
            write_reg(info, channel, ATA_REG_SECTOR, (lba>>24)&0xFF);
            write_reg(info, channel, ATA_REG_CYLINDER_LOW, (lba>>32)&0xFF);
            write_reg(info, channel, ATA_REG_CYLINDER_HIGH, (lba>>40)&0xFF);
            /* lower order bytes */
            write_reg(info, channel, ATA_REG_SECTOR_COUNT, (seccount>>0)&0xFF);
            write_reg(info, channel, ATA_REG_SECTOR, (lba>>0)&0xFF);
            write_reg(info, channel, ATA_REG_CYLINDER_LOW, (lba>>8)&0xFF);
            write_reg(info, channel, ATA_REG_CYLINDER_HIGH, (lba>>16)&0xFF);
            /* done */
            break;
        default:
            break;
    }
}

/* ================================================================= */
/*                          Status Polling                           */
/* ================================================================= */

/* layout of the status register */
#define STATUS_MASK_ERR         0x01 /* error occured for last command */
#define STATUS_MASK_DRQ         0x08 /* PIO data is ready */
#define STATUS_MASK_SRV         0x10 /* overlapped mode service request */
#define STATUS_MASK_DF          0x20 /* drive fault!! (doesn't set ERR) */
#define STATUS_MASK_RDY         0x40 /* drive is ready */
#define STATUS_MASK_BSY         0x80 /* drive is busy */

static int32_t wait_not_busy(info_t *info, int channel) {
    /* wait until BSY clears - useSTATUS register */
    extern uint64_t ticks;
    uint64_t timeout=ticks+((uint64_t)(2/SCHEDULER_INTERVAL)); /* 2 seconds */
    while (ticks<=timeout&&(read_status(info,channel)&STATUS_MASK_BSY));
    return ticks > timeout;
}

static int32_t wait_not_busy_alt(info_t *info, int channel) {
    /* wait until BSY clears - use ALTSTATUS register */
    extern uint64_t ticks;
    uint64_t timeout=ticks+((uint64_t)(2/SCHEDULER_INTERVAL)); /* 2 seconds */
    while (ticks<=timeout&&(read_status_alt(info,channel)&STATUS_MASK_BSY));
    return ticks > timeout;
}

static int32_t wait_drq_err(info_t *info, int channel) {
    /* wait until either DRQ or ERR sets */
    extern uint64_t ticks;
    uint64_t timeout=ticks+((uint64_t)(2/SCHEDULER_INTERVAL)); /* 2 seconds */
    while(ticks<=timeout &&
          !(read_status(info, channel) & STATUS_MASK_ERR) &&
          !(read_status(info, channel) & STATUS_MASK_DRQ));
    if ( (read_status(info, channel) & STATUS_MASK_DRQ) &&
        !(read_status(info, channel) & STATUS_MASK_ERR))
        return 0; /* successful */
    else if (ticks > timeout)
        return 1; /* timeout */
    else
        return 2; /* error */
}

static int32_t wait_drq_err_alt(info_t *info, int channel) {
    /* wait until either DRQ or ERR sets */
    extern uint64_t ticks;
    uint64_t timeout=ticks+((uint64_t)(2/SCHEDULER_INTERVAL)); /* 2 seconds */
    while(ticks<=timeout &&
          !(read_status_alt(info, channel) & STATUS_MASK_ERR) &&
          !(read_status_alt(info, channel) & STATUS_MASK_DRQ));
    if ( (read_status_alt(info, channel) & STATUS_MASK_DRQ) &&
        !(read_status_alt(info, channel) & STATUS_MASK_ERR))
        return 0; /* successful */
    else if (ticks > timeout)
        return 1; /* timeout */
    else
        return 2; /* error */
}

static int32_t wait_rdy_not_drq(info_t *info, int channel) {
    /* wait until DRQ clears and RDY sets */
    extern uint64_t ticks;
    uint64_t timeout=ticks+((uint64_t)(2/SCHEDULER_INTERVAL)); /* 2 seconds */
    while(ticks<=timeout &&
          ( (read_status(info, channel) & STATUS_MASK_DRQ) ||
           !(read_status(info, channel) & STATUS_MASK_RDY)));
    if (ticks > timeout)
        return 1; /* timeout */
    else
        return 0; /* successful */
}

static int32_t wait_rdy_not_drq_alt(info_t *info, int channel) {
    /* wait until DRQ clears and RDY sets */
    extern uint64_t ticks;
    uint64_t timeout=ticks+((uint64_t)(2/SCHEDULER_INTERVAL)); /* 2 seconds */
    while(ticks<=timeout &&
          ( (read_status_alt(info, channel) & STATUS_MASK_DRQ) ||
           !(read_status_alt(info, channel) & STATUS_MASK_RDY)));
    if (ticks > timeout)
        return 1; /* timeout */
    else
        return 0; /* successful */
}

/* ================================================================= */
/*                          Waiting for DRQ                          */
/* ================================================================= */

static int32_t wait_for_drq(info_t *info, uint32_t channel, uint32_t wmode) {
    int32_t err;
    uint8_t state;
    if (wmode == ATA_WMODE_IRQ) {
        /* IRQ handling */
        while(1) {
            /* sleep for IRQ */
            irq_down(info->channels[channel].irqnum);
            /* handle irq sharing */
            state = read_status(info, channel);
            if ((state & STATUS_MASK_ERR) || (state & STATUS_MASK_DRQ))
                break; /* it is our irq */
            irq_up(info->channels[channel].irqnum);
        }
        /* error occured? */
        if (state & STATUS_MASK_ERR)
            return 2;
    } else {
        /* wait until BSY clears */
        if (err = wait_not_busy_alt(info, channel))
            return err;
        /* wait until DRQ sets and ERR clears */
        if (err = wait_drq_err_alt(info, channel))
            return err;
    }
    /* done */
    return 0;
}

/* ================================================================= */
/*                           Data Transfer                           */
/* ================================================================= */

static int32_t do_pio(info_t *info,
                      uint32_t channel,
                      uint8_t *buf,
                      uint32_t bufsize,
                      uint32_t drqsize,
                      uint32_t direction,
                      uint32_t wmode) {

    int32_t err;
    int32_t i, j;

    /* loop over DRQ blocks */
    for (i = 0; i < bufsize/drqsize; i++) {
        if (direction == ATA_DIR_READ) {
            /* wait for DRQ */
            if (err = wait_for_drq(info, channel, wmode))
                return err;
            /* read block */
            for (j = 0; j < drqsize/2; j++)
                ((uint16_t *) buf)[j] = read_data2(info, channel);
        } else {
            /* wait for DRQ */
            if (err = wait_for_drq(info, channel, wmode))
                return err;
            /* write block */
            for (j = 0; j < drqsize/2; j++)
                write_data2(info, channel, ((uint16_t *) buf)[j]);
        }
        buf += drqsize;
    }

    /* wait until DRQ clears and RDY sets */
    if (err = wait_rdy_not_drq(info, channel))
        return err;

    /* done */
    return 0;

}

/* ================================================================= */
/*                             Protocols                             */
/* ================================================================= */

int32_t pio_data(info_t *info,
                 uint32_t channel,
                 uint32_t drvnum,
                 uint16_t seccount,
                 uint64_t lba,
                 int32_t  amode,
                 uint8_t  cmd,
                 uint8_t  *buf,
                 uint32_t bufsize,
                 uint32_t drqsize,
                 uint32_t direction,
                 uint32_t wmode) {

    int32_t err;

    /* 1: select the drive and set registers: */
    select_drive(info, channel, drvnum, seccount, lba, amode);

    /* 2: enable interrupts */
    if (wmode == ATA_WMODE_IRQ)
        enable_irq(info, channel);

    /* 3: send command */
    write_reg(info, channel, ATA_REG_COMMAND_STATUS, cmd);

    /* 4: do the transfer */
    err = do_pio(info, channel, buf, bufsize, drqsize, direction, wmode);

    /* 5: disable interrupts */
    disable_irq(info, channel);

    /* 6: done */
    return err;

}

/* ================================================================= */
/*                             Interface                             */
/* ================================================================= */

uint32_t ide_probe(device_t *dev, void *config) {

    int32_t i, j, k, err, is_ata;
    uint32_t progif;
    uint8_t *buf;
    pci_config_t *pci_config = config;
    resource_t *list = dev->resources.list;
    bus_t *idebus;
    device_t *t;
    class_t cls;
    reslist_t reslist = {0, NULL};

    /* create info_t structure: */
    info_t *info = (info_t *) kmalloc(sizeof(info_t));
    dev->drvreg = (uint32_t) info;
    if (info == NULL)
        return ENOMEM; /* i am sorry :D */

    /* allocate buffer */
    buf = kmalloc(sizeof(uint8_t)*512);
    if (buf == NULL) {
        kfree(info);
        return ENOMEM;
    }

    /* print message */
    printk("ATA/ATAPI IDE initialization.\n");

    /* debug info */
#if 0
    for (i = 0; i < dev->resources.count; i++) {
        printk("type: %d, base: %x\n", dev->resources.list[i]);
    }
#endif

    /* initialize the info_t structure */
    info->dev    = dev;
    info->bus    = pci_config->bus;
    info->devno  = pci_config->devno;
    info->func   = pci_config->func;
    info->master = pci_config->master;

    /* detect the I/O address ranges of the primary channel */
    if (dev->cls.progif & 1) {
        /* primary channel not in compatibility mode */
        info->channels[0].command_reg_base = list[1].data.mem.base;
        info->channels[0].command_reg_type = list[1].type;
        info->channels[0].control_reg_base = list[2].data.mem.base;
        info->channels[0].control_reg_type = list[2].type;
        info->channels[0].pci_dma_reg_base = list[5].data.mem.base;
        info->channels[0].pci_dma_reg_type = list[5].type;
        info->channels[0].irqnum           = list[0].data.irq.number;
    } else {
        /* primary channel is in compatibility mode */
        info->channels[0].command_reg_base = 0x1F0;
        info->channels[0].command_reg_type = RESOURCE_TYPE_PORT;
        info->channels[0].control_reg_base = 0x3F6;
        info->channels[0].control_reg_type = RESOURCE_TYPE_PORT;
        info->channels[0].pci_dma_reg_base = list[5].data.mem.base;
        info->channels[0].pci_dma_reg_type = list[5].type;
        info->channels[0].irqnum           = 0x0E;

    }

    /* detect the I/O address ranges of the secondary channel */
    if (dev->cls.progif & 4) {
        /* secondary channel not in compatibility mode */
        info->channels[1].command_reg_base = list[3].data.mem.base;
        info->channels[1].command_reg_type = list[3].type;
        info->channels[1].control_reg_base = list[4].data.mem.base;
        info->channels[1].control_reg_type = list[4].type;
        info->channels[1].pci_dma_reg_base = list[5].data.mem.base+8;
        info->channels[1].pci_dma_reg_type = list[5].type;
        info->channels[1].irqnum           = list[0].data.irq.number;

    } else {
        /* secondary channel is in compatibility mode */
        info->channels[1].command_reg_base = 0x170;
        info->channels[1].command_reg_type = RESOURCE_TYPE_PORT;
        info->channels[1].control_reg_base = 0x376;
        info->channels[1].control_reg_type = RESOURCE_TYPE_PORT;
        info->channels[1].pci_dma_reg_base = list[5].data.mem.base+8;
        info->channels[1].pci_dma_reg_type = list[5].type;
        info->channels[1].irqnum           = 0x0F;
    }

    /* debug information */
#if 0
    for (i = 0; i < 2; i++) {
        printk("CHANNEL %d:\n", i);
        printk("REGBASE: %x(%d), CTRLBASE: %x(%d), DMA: %x(%d), IRQ: %d\n",
               info->channels[i].command_reg_base,
               info->channels[i].command_reg_type,
               info->channels[i].control_reg_base,
               info->channels[i].control_reg_type,
               info->channels[i].pci_dma_reg_base,
               info->channels[i].pci_dma_reg_type,
               info->channels[i].irqnum);
    }
#endif

    /* loop over channels */
    for (i = 0; i < 2; i++) {
        /* software reset the bus */
        soft_reset(info, i);
        /* loop over drives attached to the bus */
        for (j = 0; j < 2; j++) {
            /* initialize drive structure */
            info->channels[i].drives[j].channel   = i;
            info->channels[i].drives[j].drvnum    = j;
            info->channels[i].drives[j].exists    = 0;
            info->channels[i].drives[j].type      = DRIVE_TYPE_ATA;
            info->channels[i].drives[j].mode      = ATA_AMODE_CHS;
            info->channels[i].drives[j].cylinders = 0;
            info->channels[i].drives[j].heads     = 0;
            /* select master */
            select_drive(info, i, j, 0, -1, ATA_AMODE_CHS);
            /* send identify command */
            write_reg(info, i, ATA_REG_COMMAND_STATUS, ATA_CMD_IDENTIFY);
            /* if status is zero, drive doesn't exist */
            if (!read_status(info, i))
                continue;
            /* wait until BSY clears */
            if (wait_not_busy(info, i))
                continue; /* timeout */
            /* if cyllow and cylhigh are zero, device maybe ATA, else ATAPI */
            if (!read_cylinder_low(info, i) && !read_cylinder_high(info, i)) {
                /* maybe ATA */
                err = wait_drq_err(info, i);
                if (err == 1) {
                    continue; /* time out */
                } else if (err == 0) {
                    /* device is ATA */
                    is_ata = 1;
                } else {
                    /* error occured... maybe ATAPI */
                    is_ata = 0;
                }
            } else {
                /* maybe ATAPI */
                is_ata = 0;
            }
            /* if ATA, read data, if not, decide whether is ATAPI or not */
            if (is_ata) {
                /* ATA device! read 256 words */
                for (k = 0; k < 256; k++)
                    ((uint16_t *) buf)[k] = read_data(info, i);
                /* wait until DRQ clears and RDY sets */
                if (wait_rdy_not_drq(info, i))
                    continue;
            } else {
                /* it is not ATA... is it ATAPI? */
                uint8_t byte0 = read_cylinder_low(info, i);
                uint8_t byte1 = read_cylinder_high(info, i);
                if ((byte0 == 0x14 && byte1 == 0xEB) ||
                    (byte0 == 0x69 && byte1 == 0x96)) {
                    /* cerainly ATAPI! send identify packet command */
                    write_reg(info, i, ATA_REG_COMMAND_STATUS,
                              ATA_CMD_IDENTIFY_PACKET);
                    /* wait until BSY clears */
                    if (wait_not_busy(info, i))
                        continue; /* timeout */
                    /* wait until DRQ sets and ERR clears */
                    if (wait_drq_err(info, i))
                        continue;
                    /* ATAPI device! read 256 words */
                    for (k = 0; k < 256; k++)
                        ((uint16_t *) buf)[k] = read_data(info, i);
                    /* wait until DRQ clears and RDY sets */
                    if (wait_rdy_not_drq(info, i))
                        continue;
                } else {
                    /* neither ATA nor ATAPI */
                    continue;
                }
            }
            /* drive exists! */
            info->channels[i].drives[j].exists = 1;
            printk("-> Channel %d - %s: ", i, j?"Slave":"Master");
            /* set drive type */
            info->channels[i].drives[j].type = !is_ata;
            printk("%s - ", is_ata?"ATA":"ATAPI");
            /* read drive model */
            strncpy(info->channels[i].drives[j].model, &buf[54], 40);
            strcuttail(info->channels[i].drives[j].model, ' ');
            printk("%a", 0x0E);
            printk("%s", info->channels[i].drives[j].model);
            /* read drive serial number */
            strncpy(info->channels[i].drives[j].serial, &buf[20], 20);
            strcuttail(info->channels[i].drives[j].serial, ' ');
            /*printk(" %s", info->channels[i].drives[j].serial);*/
            printk("%a - ", 0x0F);
            /* read capabilities */
            /* read word 49 (bit 8 (dma support)), this means
             * we need bit 0 of byte (49*2) [notice the endianness].
             */
            info->channels[i].drives[j].dma = (buf[98]>>0)&1;
            printk("%s", info->channels[i].drives[j].dma?"DMA":"PIO");
            /* read lba support */
            /* read word 49 (bit 9 (lba support)), this means
             * we need bit 1 of byte (49*2) [notice the endianness]
             * we also read word 83 bit 10 (byte 166 bit 2) to
             * check for LBA48 support.
             */
            if (is_ata) {
                info->channels[i].drives[j].mode = ((buf[98]>>1)&1) +
                                                   ((buf[166]>>2)&1);
                printk(" - %s ",
                       info->channels[i].drives[j].mode==2?"LBA48":(
                       info->channels[i].drives[j].mode==1?"LBA28":"CHS"));
            }
            /* read disk size */
            if (is_ata) {
                uint64_t size;
                /* read CHS geometry (obsolete) */
                /* cylinders (word 1): */
                info->channels[i].drives[j].cylinders = (buf[ 3]<<0) |
                                                        (buf[ 2]<<8);
                /* heads (word 3): */
                info->channels[i].drives[j].heads     = (buf[ 7]<<0) |
                                                        (buf[ 6]<<8);
                /* sectors per track (word 6): */
                info->channels[i].drives[j].sectors   = (buf[13]<<0) |
                                                        (buf[12]<<8);
#if 0
                printk("\n Cyl: %d, Heads: %d, Sectors: %d\n",
                    info->channels[i].drives[j].cylinders,
                    info->channels[i].drives[j].heads,
                    info->channels[i].drives[j].sectors
                );
#endif
                /* get total number of sectors */
                switch(info->channels[i].drives[j].mode) {
                    case ATA_AMODE_CHS:
                        info->channels[i].drives[j].allsectors =
                            ((uint64_t)info->channels[i].drives[j].cylinders)*
                            ((uint64_t)info->channels[i].drives[j].heads    )*
                            ((uint64_t)info->channels[i].drives[j].sectors  );
                        break;
                    case ATA_AMODE_LBA28:
                        info->channels[i].drives[j].allsectors =
                            (buf[121]<< 0)|
                            (buf[120]<< 8)|
                            (buf[123]<<16)|
                            (buf[122]<<24);
                        break;
                    case ATA_AMODE_LBA48:
                        info->channels[i].drives[j].allsectors =
                            (((uint64_t)buf[201])<< 0)|
                            (((uint64_t)buf[200])<< 8)|
                            (((uint64_t)buf[203])<<16)|
                            (((uint64_t)buf[202])<<24)|
                            (((uint64_t)buf[205])<<32)|
                            (((uint64_t)buf[204])<<40)|
                            (((uint64_t)buf[207])<<48)|
                            (((uint64_t)buf[206])<<56);
                        break;
                    default:
                        break;
                }
                /* print disk size */
                size = (info->channels[i].drives[j].allsectors*512)/1024/1024;
                if (size < 1024)
                    printk("(%dMB)", (uint32_t) size);
                else
                    printk("(%dGB)", (uint32_t) (size/1024) + (size%1024?1:0));
            }
            /* print buffer */
#if 0
            printk("\n");
            for (k = 0; k < 512; k++)
                printk("%c", buf[k]);
#endif
            /* done: */
            printk("\n");
        }
    }

    /* create IDE bus */
    dev_mkbus(&idebus, BUS_IDE, dev);

    /* loop over drives and add them to the system */
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            if (info->channels[i].drives[j].exists) {
                if (info->channels[i].drives[j].type == DRIVE_TYPE_ATA) {
                    /* ATA drive */
                    cls.bus    = BUS_IDE;
                    cls.base   = BASE_ATA_DISK;
                    cls.sub    = SUB_ATA_DISK_GENERIC;
                    cls.progif = IF_ANY;
                    dev_add(&t,idebus,cls,reslist,&info->channels[i].drives[j]);
                } else {
                    /* ATAPI drive */
                    cls.bus    = BUS_IDE;
                    cls.base   = BASE_ATAPI_CDROM;
                    cls.sub    = SUB_ATAPI_CDROM_GENERIC;
                    cls.progif = IF_ANY;
                    dev_add(&t,idebus,cls,reslist,&info->channels[i].drives[j]);
                }
            }
        }
    }
    /* free the buffer */
    kfree(buf);

    /* done */
    return ESUCCESS;
}

uint32_t ide_read(device_t *dev, uint64_t off, uint32_t size, char *buff){
    return ESUCCESS;
}

uint32_t ide_write(device_t *dev, uint64_t off, uint32_t size,char *buff){
    return ESUCCESS;
}

uint32_t ide_ioctl(device_t *dev, uint32_t cmd, void *data) {
    info_t *info = (info_t *) dev->drvreg;
    ata_req_t *req = data;
    switch (req->protocol) {
        case ATA_PROTO_PIO:
            return pio_data(info, req->channel, req->drvnum, req->seccount,
                            req->lba, req->amode, req->cmd, req->buf,
                            req->bufsize, req->drqsize, req->direction,
                            req->wmode);
        case ATA_PROTO_PACKET:
            return 2;
        default:
            return 2; /* error */
    }
}

uint32_t ide_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
