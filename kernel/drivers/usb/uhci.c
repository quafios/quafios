/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> USB Subsystem: UHCI Device Driver                | |
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
uint32_t uhci_probe(device_t *, void *);
uint32_t uhci_read (device_t *, uint64_t, uint32_t, char *);
uint32_t uhci_write(device_t *, uint64_t, uint32_t, char *);
uint32_t uhci_ioctl(device_t *, uint32_t, void *);
uint32_t uhci_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_PCI, BASE_PCI_SERIALBUS,
     SUB_PCI_SERIALBUS_USB, IF_PCI_SERIALBUS_USB_UHCI}
};

/* driver_t structure that identifies this driver: */
driver_t uhci_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "uhci",
    /* probe:     */ uhci_probe,
    /* read:      */ uhci_read,
    /* write:     */ uhci_write,
    /* ioctl:     */ uhci_ioctl,
    /* irq:       */ uhci_irq
};

#define TO_PHYS(addr) (arch_vmpage_getAddr(NULL,(uint32_t)addr)\
                       +(((uint32_t)addr)&0xFFF))

/* frame list pointer */
typedef struct flptr {
    unsigned int T     : 1;  /* terminate */
    unsigned int Q     : 1;  /* 1: QH, 0: TD */
    unsigned int RES   : 2;  /* reserved */
    unsigned int FLP   : 28; /* frame list pointer */
} __attribute__((packed)) flptr_t;

/* queue head */
typedef struct qh {
    unsigned int T1    : 1;  /* terminate */
    unsigned int Q1    : 1;  /* 1: QH, 0: TD */
    unsigned int RES1  : 2;  /* reserved */
    unsigned int QHLP  : 28; /* frame list pointer */
    unsigned int T2    : 1;  /* terminate */
    unsigned int Q2    : 1;  /* 1: QH, 0: TD */
    unsigned int RES2  : 2;  /* reserved */
    unsigned int QELP  : 28; /* frame list pointer */
} __attribute__((packed)) qh_t;

/* transfer descriptor */
typedef struct td {
    unsigned int T      : 1;
    unsigned int Q      : 1;
    unsigned int Vf     : 1;
    unsigned int RES1   : 1;  /* reserved */
    unsigned int LP     : 28; /* link pointer */
    unsigned int ActLen : 11; /* actual length */
    unsigned int RES2   : 5;
    unsigned int Status : 8;
    unsigned int IOC    : 1;
    unsigned int ISO    : 1;
    unsigned int LS     : 1;
    unsigned int C_ERR  : 2;
    unsigned int SPD    : 1;
    unsigned int RES3   : 2;
    unsigned int PID    : 8;
    unsigned int Addr   : 7;
    unsigned int EndPt  : 4;
    unsigned int D      : 1;
    unsigned int RES4   : 1;
    unsigned int MaxLen : 11;
    unsigned int BufPtr : 32;
} __attribute__((packed)) td_t;

/* info structure */
typedef struct info {
    device_t *dev;
    uint32_t iotype;
    uint32_t iobase;
    uint32_t irqnum;
    flptr_t *framelist;
    qh_t    *qh_first;
    qh_t    *qh_last;
} info_t;

/* ================================================================= */
/*                           Register I/O                            */
/* ================================================================= */

/* basic UHCI registers */
#define USBCMD          0x00
#define USBSTS          0x02
#define USBINTR         0x04
#define FRNUM           0x06
#define FLBASEADD       0x08
#define SOFMOD          0x0C
#define PORTSC1         0x10
#define PORTSC2         0x12

/* USBCMD register */
#define RS              0x01 /* RUN(1)/STOP(0) */
#define HCRESET         0x02 /* host controller reset */
#define GRESET          0x04 /* global USB reset */
#define EGSM            0x08 /* enter global suspend mode */
#define FGR             0x10 /* force global resume */
#define SWDBG           0x20 /* enable debug mode */
#define CF              0x40 /* configure flag */
#define MAXP            0x80 /* max packet (1: 64 bytes, 0: 32 bytes)

/* USBSTS register */
#define USBINT          0x01 /* usb interrupt? set on completion of TD */
#define USBERRINT       0x02 /* usb error interrupt? a TD has error */
#define RESUME          0x04 /* resume signal received */
#define HSERR           0x08 /* host system error */
#define HCERR           0x10 /* host controller error */
#define HCHALTED        0x20 /* host controller halted */

/* USBINTR register */
#define TCIE            0x01 /* timeout/CRC interrupt enable */
#define RIE             0x02 /* resume interrupt enable */
#define IOCE            0x04 /* interrupt on complete enable */
#define SPIE            0x08 /* short packet interrupt enable */

/* PORTSC registers */
#define CCS             0x0001 /* current connect status */
#define CSC             0x0002 /* connect status change */
#define PED             0x0004 /* port enabled/disabled */
#define PEDC            0x0008 /* port enable/disable change */
#define LINESTATUS      0x0030 /* line status (D+ and D-) */
#define RD              0x0040 /* resume detect */
#define LSDA            0x0100 /* low-speed device attached */
#define PR              0x0200 /* port reset */
#define SUSPEND         0x1000 /* suspend */

uint16_t uhci_read_cmd(info_t *info) {
    return ioread(2, info->iotype, info->iobase, USBCMD);
}

void uhci_write_cmd(info_t *info, uint16_t val) {
    iowrite(2, info->iotype, val, info->iobase, USBCMD);
}

uint16_t uhci_read_sts(info_t *info) {
    return ioread(2, info->iotype, info->iobase, USBSTS);
}

void uhci_write_sts(info_t *info, uint16_t val) {
    iowrite(2, info->iotype, val, info->iobase, USBSTS);
}

uint16_t uhci_read_intr(info_t *info) {
    return ioread(2, info->iotype, info->iobase, USBINTR);
}

void uhci_write_intr(info_t *info, uint16_t val) {
    iowrite(2, info->iotype, val, info->iobase, USBINTR);
}

uint16_t uhci_read_frnum(info_t *info) {
    return ioread(2, info->iotype, info->iobase, FRNUM);
}

void uhci_write_frnum(info_t *info, uint16_t val) {
    iowrite(2, info->iotype, val, info->iobase, FRNUM);
}

uint32_t uhci_read_flbaseadd(info_t *info) {
    return ioread(4, info->iotype, info->iobase, FLBASEADD);
}

void uhci_write_flbaseadd(info_t *info, uint32_t val) {
    iowrite(4, info->iotype, val, info->iobase, FLBASEADD);
}

uint8_t uhci_read_sofmod(info_t *info) {
    return ioread(1, info->iotype, info->iobase, SOFMOD);
}

void uhci_write_sofmod(info_t *info, uint8_t val) {
    iowrite(1, info->iotype, val, info->iobase, SOFMOD);
}

uint16_t uhci_read_portsc1(info_t *info) {
    return ioread(2, info->iotype, info->iobase, PORTSC1);
}

void uhci_write_portsc1(info_t *info, uint16_t val) {
    iowrite(2, info->iotype, val, info->iobase, PORTSC1);
}

uint16_t uhci_read_portsc2(info_t *info) {
    return ioread(2, info->iotype, info->iobase, PORTSC2);
}

void uhci_write_portsc2(info_t *info, uint16_t val) {
    iowrite(2, info->iotype, val, info->iobase, PORTSC2);
}

uint16_t uhci_read_portsc(info_t *info, int32_t port) {
    return ioread(2, info->iotype, info->iobase, PORTSC1+(port<<1));
}

void uhci_write_portsc(info_t *info, int32_t port, uint16_t val) {
    iowrite(2, info->iotype, val, info->iobase, PORTSC1+(port<<1));
}

/* ================================================================= */
/*                          Root Hub Emulation                       */
/* ================================================================= */

int32_t uhci_root_hub_ctrl_msg(info_t *info, usb_cmd_t *cmd) {
    if (cmd->requesttype == 0x00 && cmd->request == CLEAR_FEATURE) {
        /* standard CLEAR_FEATURE request for device */
        return 0;
    } else if (cmd->requesttype == 0x01 && cmd->request == CLEAR_FEATURE) {
        /* standard CLEAR_FEATURE request for interface */
        return 0;
    } else if (cmd->requesttype == 0x02 && cmd->request == CLEAR_FEATURE) {
        /* standard CLEAR_FEATURE request for endpoint */
        return 0;
    } else if (cmd->requesttype == 0x80 && cmd->request == GET_CONFIGURATION) {
        /* standard GET_CONFIGURATION request */
        return 0;
    } else if (cmd->requesttype == 0x80 && cmd->request == GET_DESCRIPTOR) {
        /* standard GET_DESCRIPTOR request */
        return 0;
    } else if (cmd->requesttype == 0x81 && cmd->request == GET_INTERFACE) {
        /* standard GET_INTERFACE request */
        return 0;
    } else if (cmd->requesttype == 0x80 && cmd->request == GET_STATUS) {
        /* standard GET_STATUS request for device */
        return 0;
    } else if (cmd->requesttype == 0x81 && cmd->request == GET_STATUS) {
        /* standard GET_STATUS request for interface */
        return 0;
    } else if (cmd->requesttype == 0x82 && cmd->request == GET_STATUS) {
        /* standard GET_STATUS request for endpoint */
        return 0;
    } else if (cmd->requesttype == 0x00 && cmd->request == SET_ADDRESS) {
        /* standard SET_ADDRESS request */
        return 0;
    } else if (cmd->requesttype == 0x00 && cmd->request == SET_CONFIGURATION) {
        /* standard SET_CONFIGURATION request */
        return 0;
    } else if (cmd->requesttype == 0x00 && cmd->request == SET_DESCRIPTOR) {
        /* standard SET_DESCRIPTOR request */
        return 0;
    } else if (cmd->requesttype == 0x00 && cmd->request == SET_FEATURE) {
        /* standard SET_FEATURE request for device */
        return 0;
    } else if (cmd->requesttype == 0x01 && cmd->request == SET_FEATURE) {
        /* standard SET_FEATURE request for interface */
        return 0;
    } else if (cmd->requesttype == 0x02 && cmd->request == SET_FEATURE) {
        /* standard SET_FEATURE request for endpoint */
        return 0;
    } else if (cmd->requesttype == 0x01 && cmd->request == SET_INTERFACE) {
        /* standard SET_INTERFACE request */
        return 0;
    } else if (cmd->requesttype == 0x82 && cmd->request == SYNCH_FRAME) {
        /* standard SYNCH_FRAME request */
        return 0;
    } else if (cmd->requesttype == 0x20 && cmd->request == CLEAR_FEATURE) {
        /* ClearHubFeature request */
        return 0;
    } else if (cmd->requesttype == 0x23 && cmd->request == CLEAR_FEATURE) {
        /* ClearPortFeature request */
        uint16_t portsc = uhci_read_portsc(info, cmd->index);
        if (cmd->value == PORT_ENABLE) {
            portsc &= ~PED & 0xFFF5;
        } else if (cmd->value == PORT_SUSPEND) {
            portsc &= ~SUSPEND & 0xFFF5;
        } else if (cmd->value == C_PORT_CONNECTION) {
            portsc = (portsc & 0xFFF5) | CSC;
        } else if (cmd->value == C_PORT_ENABLE) {
            portsc = (portsc & 0xFFF5) | PEDC;
        }
        uhci_write_portsc(info, cmd->index, portsc);
        return 0;
    } else if (cmd->requesttype == 0xA3 && cmd->request == GET_STATE) {
        /* GetBusState request */
        return 0;
    } else if (cmd->requesttype == 0xA0 && cmd->request == GET_DESCRIPTOR) {
        /* GetHubDescriptor request */
        hub_descriptor_t *hubdesc = (hub_descriptor_t *) cmd->data;
        hubdesc->bDescLength = 9;
        hubdesc->bDescriptorType = HUB_DESCRIPTOR;
        hubdesc->bNbrPorts = 2;
        hubdesc->wHubCharacteristics = 0x09;
        hubdesc->bPwrOn2PwrGood = 1;
        hubdesc->bHubContrCurrent = 100;
        hubdesc->DeviceRemovable_PortPwrCtrlMask[0] = 0;
        hubdesc->DeviceRemovable_PortPwrCtrlMask[1] = 0xFF;
        return 9;
    } else if (cmd->requesttype == 0xA0 && cmd->request == GET_STATUS) {
        /* GetHubStatus request */
        return 0;
    } else if (cmd->requesttype == 0xA3 && cmd->request == GET_STATUS) {
        /* GetPortStatus request */
        uint16_t portsc = uhci_read_portsc(info, cmd->index);
        wPortStatus_t *status = (wPortStatus_t *) &((char *)cmd->data)[0];
        wPortChange_t *change = (wPortChange_t *) &((char *)cmd->data)[2];
        status->port_connection = portsc & CCS ? 1 : 0;
        status->port_enable = portsc & PED ? 1 : 0;
        status->port_suspend = portsc & SUSPEND ? 1 : 0;
        status->port_over_current = 0;
        status->port_reset = portsc & PR ? 1 : 0;
        status->reserved1 = 0;
        status->port_power = 1;
        status->port_low_speed = portsc & LSDA ? 1 : 0;
        status->reserved2 = 0;
        change->c_port_connection = portsc & CSC ? 1 : 0;
        change->c_port_enable = portsc & PEDC ? 1 : 0;
        change->c_port_suspend = 0;
        change->c_port_over_current = 0;
        change->c_port_reset = 0;
        change->reserved = 0;
        return 4;
    } else if (cmd->requesttype == 0x20 && cmd->request == SET_DESCRIPTOR) {
        /* SetHubDescriptor request */
        return 0;
    } else if (cmd->requesttype == 0x20 && cmd->request == SET_FEATURE) {
        /* SetHubFeature request */
        return 0;
    } else if (cmd->requesttype == 0x23 && cmd->request == SET_FEATURE) {
        /* SetPortFeature request */
        uint16_t portsc = uhci_read_portsc(info, cmd->index);
        if (cmd->value == PORT_RESET) {
            portsc = (portsc & 0xFFF5) | PED | PR;
        } else if (cmd->value == PORT_SUSPEND) {
            portsc = (portsc & 0xFFF5) | SUSPEND;
        }
        uhci_write_portsc(info, cmd->index, portsc);
        if (cmd->value == PORT_RESET) {
            sleep(10);
            uhci_write_portsc(info, cmd->index, portsc & 0xFFF5 & ~PR);
        }
        return 0;
    } else {
        return -EIO;
    }
}

/* ================================================================= */
/*                            Transactions                           */
/* ================================================================= */

int32_t uhci_transfer(info_t *info, int32_t count, transaction_t *trans) {

    /* schedule & perform transfer */
    td_t *tds;
    qh_t *qh;
    char **bufs;
    int32_t i, j;
    int32_t actual = 0;
    uint8_t status;

    /* allocate transfer descriptors */
    tds = kmalloc(sizeof(td_t)*count);
    for (i = 0; i < count-1; i++)
        tds[i].T = 0; /* issue page fault */
    tds[i].T = 1;

    /* allocate buffer list */
    bufs = kmalloc(sizeof(char *) * count);

    /* allocate queue head */
    qh = kmalloc(16);

    /* initialize all TDs */
    for (i = 0; i < count; i++) {
        /* allocate buffer for current transaction */
        bufs[i] = kmalloc(trans[i].pktsize);
        for (j = 0; j < trans[i].pktsize; j++)
            bufs[i][j] = trans[i].databuf[j];
        /* initialize TD of current transaction */
        tds[i].Q      = 0;  /* next entry is TD */
        tds[i].Vf     = 1;  /* depth first */
        tds[i].RES1   = 0;
        tds[i].LP     = TO_PHYS(&tds[i+1])>>4;
        tds[i].ActLen = 0;
        tds[i].RES2   = 0;
        tds[i].Status = 0x80;
        tds[i].IOC    = 0; /* interrupt on completion */
        tds[i].ISO    = 0; /* isochronous select */
        tds[i].LS     = trans[i].lowspeed;
        tds[i].C_ERR  = 1;
        tds[i].SPD    = 0; /* short packet detect */
        tds[i].RES3   = 0;
        tds[i].PID    = trans[i].tokpid;
        tds[i].Addr   = trans[i].addr;
        tds[i].EndPt  = trans[i].endpoint;
        tds[i].D      = trans[i].datapid == PID_DATA1;
        tds[i].RES4   = 0;
        tds[i].MaxLen = trans[i].pktsize-1;
        tds[i].BufPtr = TO_PHYS(bufs[i]);
    }

    /* initialize QH */
    qh->T1   = 0;
    qh->Q1   = 1;
    qh->RES1 = 0;
    qh->QHLP = 0;
    qh->T2   = 0;
    qh->Q2   = 0;
    qh->RES2 = 0;
    qh->QELP = TO_PHYS(&tds[0])>>4;

    /* write back */
    writeback();

    /* add QH to linked list of current frame */
    qh->QHLP = info->qh_first->QHLP;
    info->qh_first->QHLP = TO_PHYS(qh)>>4;

    /* write back again */
    writeback();

    /* wait for transfer to finish */
    for (i = 0; i < count; i++) {
        while ((status = tds[i].Status) & 0x80)
            writeback();
        if (status)
            break;
        if (trans[i].calc)
            actual += (tds[i].ActLen+1)&0x7FF;
    }

    /* remove QH from the linked list */
    info->qh_first->QHLP = qh->QHLP;

    /* rewrite and deallocate buffers */
    for (i = 0; i < count; i++) {
        for (j = 0; j < trans[i].pktsize; j++)
            trans[i].databuf[j] = bufs[i][j];
        kfree(bufs[i]);
    }

    /* deallocate buffer list */
    kfree (bufs);

    /* deallocate QH */
    kfree(qh);

    /* deallocate TDs */
    kfree(tds);

    /* done */
    if (status)
        return -EIO;
    else
        return actual;
}

/* ================================================================= */
/*                             Interface                             */
/* ================================================================= */

uint32_t uhci_probe(device_t *dev, void *config) {

    resource_t *list = dev->resources.list;
    pci_config_t *pci_config = (pci_config_t *) config;
    int i;

    /* create info_t structure: */
    info_t *info = (info_t *) kmalloc(sizeof(info_t));
    dev->drvreg = (uint32_t) info;
    if (info == NULL)
        return ENOMEM; /* i am sorry :D */

    /* inform user of our progress */
    printk("%aUSB%a: ", 0x0A, 0x0F);
    printk("Universal host controller interface (UHCI) on PCI.\n");

    /* disable USB legacy support and initialize companion EHCI controllers */
    ehci_init_companion(pci_config->bus, pci_config->devno, pci_config->func);

    /* store data in info structure */
    info->dev = dev;
    info->iotype = list[5].type;
    info->iobase = list[5].data.port.base;
    info->irqnum = list[0].data.irq.number;
    info->qh_first = kmalloc(16);
    info->qh_last  = kmalloc(16);
    info->qh_first->T1   = 0; /* QH next pointer valid */
    info->qh_first->Q1   = 1; /* QH next pointer is QH */
    info->qh_first->RES1 = 0; /* reserved */
    info->qh_first->QHLP = TO_PHYS(info->qh_last)>>4;
    info->qh_first->T2   = 1; /* QH doesn't contain elements */
    info->qh_first->Q2   = 0;
    info->qh_first->RES2 = 0;
    info->qh_first->QELP = 0;
    info->qh_last ->T1   = 1; /* QH next pointer invalid */
    info->qh_last ->Q1   = 0;
    info->qh_last ->RES1 = 0;
    info->qh_last ->QHLP = 0;
    info->qh_last ->T2   = 1; /* QH doesn't contain elements */
    info->qh_last ->Q2   = 0;
    info->qh_last ->RES2 = 0;
    info->qh_last ->QELP = 0;
    info->framelist = kmalloc(sizeof(flptr_t)*1024);

    /* initialize frame list */
    for (i = 0; i < 1024; i++) {
        info->framelist[i].T   = 0;
        info->framelist[i].Q   = 1;
        info->framelist[i].RES = 0;
        info->framelist[i].FLP = TO_PHYS(info->qh_first)>>4;
    }

    /* write back caches */
    writeback();

    /* stop HC */
    if (uhci_read_cmd(info) & RS) {
        uhci_write_cmd(info, uhci_read_cmd(info) & ~RS);
        while (!(uhci_read_sts(info) & HCHALTED));
    }

    /* reset HC */
    uhci_write_cmd(info, HCRESET); /* host controller reset */
    while (uhci_read_cmd(info) & HCRESET);
    uhci_write_cmd(info, GRESET);  /* global reset */
    sleep(10);
    uhci_write_cmd(info, 0);
    uhci_write_cmd(info, EGSM | FGR);
    sleep(20);
    uhci_write_cmd(info, 0);

    /* initialize HC */
    uhci_write_intr(info, 0);
    uhci_write_frnum(info, 0);
    uhci_write_flbaseadd(info, TO_PHYS(info->framelist));
    uhci_write_sofmod(info, 64);
    uhci_write_cmd(info, RS);

#if 0
    printk("cmd:     %x\n", uhci_read_cmd(info));
    printk("sts:     %x\n", uhci_read_sts(info));
    printk("intr:    %x\n", uhci_read_intr(info));
    printk("frnum:   %x\n", uhci_read_frnum(info));
    printk("flbase:  %x\n", uhci_read_flbaseadd(info));
    printk("sofmod:  %x\n", uhci_read_sofmod(info));
    printk("portsc1: %x\n", uhci_read_portsc1(info));
    printk("portsc1: %x\n", uhci_read_portsc2(info));

    while (1) {
        printk("frnum:   %x\n", uhci_read_frnum(info));
    }
#endif

    /* register our UHCI at the USB core system of Quafios */
    usb_reg_hci(dev);

    /* done */
    return ESUCCESS;
}

uint32_t uhci_read(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    return ESUCCESS;
}

uint32_t uhci_write(device_t *dev, uint64_t off, uint32_t size,char *buff) {
    return ESUCCESS;
}

uint32_t uhci_ioctl(device_t *dev, uint32_t cmd, void *data) {
    info_t *info = (info_t *) dev->drvreg;
    if (cmd == USB_CMD_HUBCTRL) {
        /* root hub */
        return uhci_root_hub_ctrl_msg(info, data);
    } else {
        return uhci_transfer(info, cmd, data);
    }
}

uint32_t uhci_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
