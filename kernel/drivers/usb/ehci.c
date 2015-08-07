/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> USB Subsystem: EHCI Device Driver                | |
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
#include <arch/page.h>
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
uint32_t ehci_probe(device_t *, void *);
uint32_t ehci_read (device_t *, uint64_t, uint32_t, char *);
uint32_t ehci_write(device_t *, uint64_t, uint32_t, char *);
uint32_t ehci_ioctl(device_t *, uint32_t, void *);
uint32_t ehci_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_PCI, BASE_PCI_SERIALBUS,
     SUB_PCI_SERIALBUS_USB, IF_PCI_SERIALBUS_USB_EHCI}
};

/* driver_t structure that identifies this driver: */
driver_t ehci_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "ehci",
    /* probe:     */ ehci_probe,
    /* read:      */ ehci_read,
    /* write:     */ ehci_write,
    /* ioctl:     */ ehci_ioctl,
    /* irq:       */ ehci_irq
};

#define TO_PHYS(addr)  to_phys((uint32_t)addr)

/* frame list element pointer */
typedef struct flep {
    unsigned int T     : 1;  /* terminate */

#define TYP_ITD         0
#define TYP_QH          1
#define TYP_SITD        2
#define TYP_FSTN        3
    unsigned int Typ   : 2;  /* type */
    unsigned int Zero  : 2;  /* reserved */
    unsigned int FLLP  : 27; /* frame list link pointer */
}  __attribute__((packed)) flep_t;

/* queue head */
typedef struct qh {
    unsigned int T           : 1;
    unsigned int Typ         : 2;
    unsigned int Zero1       : 2;
    unsigned int QHHLP       : 27;

    unsigned int DevAddr     : 7;
    unsigned int I           : 1;
    unsigned int EndPt       : 4;
    unsigned int EPS         : 2;
    unsigned int dtc         : 1;
    unsigned int H           : 1;
    unsigned int MaxPacket   : 11;
    unsigned int C           : 1;
    unsigned int RL          : 4;

    unsigned int uFrSmask    : 8;
    unsigned int uFrCmask    : 8;
    unsigned int HubAddr     : 7;
    unsigned int PortNum     : 7;
    unsigned int Mult        : 2;

    unsigned int Zero2       : 5;
    unsigned int curQTDP     : 27;

    unsigned int nextT       : 1;
    unsigned int Zero3       : 4;
    unsigned int nextQTDP    : 27;

    unsigned int AltNextT    : 1;
    unsigned int NakCnt      : 4;
    unsigned int AltNextQTDP : 27;

    unsigned int Status      : 8;
    unsigned int PIDCode     : 2;
    unsigned int Cerr        : 2;
    unsigned int C_Page      : 3;
    unsigned int ioc         : 1;
    unsigned int TotalBytes  : 15;
    unsigned int dt          : 1;

    unsigned int CurOffset   : 12;
    unsigned int BufPtr0     : 20;

    unsigned int CProgMask   : 8;
    unsigned int Reserved1   : 4;
    unsigned int BufPtr1     : 20;

    unsigned int FrameTag    : 5;
    unsigned int SBytes      : 7;
    unsigned int BufPtr2     : 20;

    unsigned int Reserved2   : 12;
    unsigned int BufPtr3     : 20;

    unsigned int Reserved3   : 12;
    unsigned int BufPtr4     : 20;

    unsigned int ExtBufPtr0  : 32;
    unsigned int ExtBufPtr1  : 32;
    unsigned int ExtBufPtr2  : 32;
    unsigned int ExtBufPtr3  : 32;
    unsigned int ExtBufPtr4  : 32;

} __attribute__((packed)) qh_t;

typedef volatile struct qtd {
    unsigned int nextT       : 1;
    unsigned int Zero1       : 4;
    unsigned int nextQTDP    : 27;

    unsigned int AltNextT    : 1;
    unsigned int Zero2       : 4;
    unsigned int AltNextQTDP : 27;

    unsigned int Status      : 8;
    unsigned int PIDCode     : 2;
    unsigned int Cerr        : 2;
    unsigned int C_Page      : 3;
    unsigned int ioc         : 1;
    unsigned int TotalBytes  : 15;
    unsigned int dt          : 1;

    unsigned int CurOffset   : 12;
    unsigned int BufPtr0     : 20;

    unsigned int Reserved1   : 12;
    unsigned int BufPtr1     : 20;

    unsigned int Reserved2   : 12;
    unsigned int BufPtr2     : 20;

    unsigned int Reserved3   : 12;
    unsigned int BufPtr3     : 20;

    unsigned int Reserved4   : 12;
    unsigned int BufPtr4     : 20;

    unsigned int ExtBufPtr0  : 32;
    unsigned int ExtBufPtr1  : 32;
    unsigned int ExtBufPtr2  : 32;
    unsigned int ExtBufPtr3  : 32;
    unsigned int ExtBufPtr4  : 32;

    unsigned int Padding1    : 32;
    unsigned int Padding2    : 32;
    unsigned int Padding3    : 32;

} __attribute__((packed)) qtd_t;

/* capability register set */
typedef struct capregs {
    uint8_t  caplength;
    uint8_t  reserved;
    uint16_t hciversion;
    uint32_t hcsparams;        /* structural parameters */

#define ADDRESS64       0x000001
#define PROGFRLISTFLAG  0x000002
#define ASYNCPARK       0x000004
#define ISOTHRESHOLD    0x0000F0
#define EECP            0x00FF00
    uint32_t hccparams;        /* capability parameters */
    uint64_t hcsp_portroute;   /* companion port route description */
} __attribute__((packed)) capregs_t;

/* operational register set */
typedef struct opregs {
#define RS              0x000001
#define HCRESET         0x000002
#define FRLISTSIZE      0x00000C
#define PERIODICENABLE  0x000010
#define ASYNCENABLE     0x000020
#define INTASYNCENABLE  0x000040
#define LIGHTRESET      0x000080
#define ASYNCPARKCOUNT  0x000300
#define ASYNCPARKENABLE 0x000800
#define INTTHRESHOLD    0x7F0000

#define FRLISTSIZE1024  0x000000
#define FRLISTSIZE512   0x000004
#define FRLISTSIZE256   0x000008

#define INTTHRESHOLD1   0x010000
#define INTTHRESHOLD2   0x020000
#define INTTHRESHOLD4   0x040000
#define INTTHRESHOLD8   0x080000
#define INTTHRESHOLD16  0x100000
#define INTTHRESHOLD32  0x200000
#define INTTHRESHOLD64  0x400000
    uint32_t usbcmd;           /* usb command */

#define USBINT          0x000001
#define USBERRINT       0x000002
#define PORTCHANGE      0x000004
#define FRLISTROLLOVER  0x000008
#define HOSTERROR       0x000010
#define INTASYNC        0x000020
#define HCHALTED        0x001000
#define RECLAMATION     0x002000
#define PERIODICSTATUS  0x004000
#define ASYNCSTATUS     0x008000
    uint32_t usbsts;           /* usb status */
    uint32_t usbintr;          /* usb interrupt enable */
    uint32_t frindex;          /* usb frame index */
    uint32_t ctrldssegment;    /* 4G segment selector */
    uint32_t periodiclistbase; /* periodic list base */
    uint32_t asynclistaddr;    /* asynchronous list address */
    uint32_t reserved[9];      /* reserved */
    uint32_t configflag;       /* configured flag register */

#define CCS             0x000001
#define CSC             0x000002
#define PED             0x000004
#define PEDC            0x000008
#define OCA             0x000010
#define OCC             0x000020
#define FPR             0x000040
#define SUSPEND         0x000080
#define PR              0x000100
#define LS              0x000C00
#define PP              0x001000
#define PO              0x002000
#define PIC             0x00C000
#define PTC             0x0F0000
#define WKCNNT_E        0x100000
#define WKDSCNNT_E      0x200000
#define WKOC_E          0x400000
    uint32_t portsc[15];       /* port status/control */
} __attribute__((packed)) opregs_t;

/* companion controllers */
typedef struct companion {
    struct companion *next;
    device_t *dev;
    pci_config_t pci_config;
} companion_t;
companion_t *compfirst = NULL;

/* info structure */
typedef struct info {
    device_t  *dev;
    uint32_t   iotype;
    uint32_t   iobase;
    uint32_t   irqnum;
    capregs_t *capregs;
    uint32_t   caplen;
    opregs_t  *opregs;
    uint32_t   portcount;
    qh_t      *qhfirst;
    qh_t      *qhlast;
} info_t;

/* ================================================================= */
/*                          Root Hub Emulation                       */
/* ================================================================= */

int32_t ehci_root_hub_ctrl_msg(info_t *info, usb_cmd_t *cmd) {
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
        uint32_t portindex = cmd->index - 1;
        uint32_t portsc = info->opregs->portsc[portindex];
        if (cmd->value == PORT_ENABLE) {
            portsc &= ~PED & 0xFFFFFFF5;
        } else if (cmd->value == PORT_SUSPEND) {
            portsc &= ~SUSPEND & 0xFFFFFFF5;
        } else if (cmd->value == C_PORT_CONNECTION) {
            portsc = (portsc & 0xFFFFFFF5) | CSC;
        } else if (cmd->value == C_PORT_ENABLE) {
            portsc = (portsc & 0xFFFFFFF5) | PEDC;
        }
        info->opregs->portsc[portindex] = portsc;
        return 0;
    } else if (cmd->requesttype == 0xA3 && cmd->request == GET_STATE) {
        /* GetBusState request */
        return 0;
    } else if (cmd->requesttype == 0xA0 && cmd->request == GET_DESCRIPTOR) {
        /* GetHubDescriptor request */
        hub_descriptor_t *hubdesc = (hub_descriptor_t *) cmd->data;
        hubdesc->bDescLength = 9;
        hubdesc->bDescriptorType = HUB_DESCRIPTOR;
        hubdesc->bNbrPorts = info->portcount;
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
        uint32_t portindex = cmd->index - 1;
        uint32_t portsc = info->opregs->portsc[portindex];
        wPortStatus_t *status = (wPortStatus_t *) &((char *)cmd->data)[0];
        wPortChange_t *change = (wPortChange_t *) &((char *)cmd->data)[2];
        status->port_connection = portsc & CCS ? 1 : 0;
        status->port_enable = portsc & PED ? 1 : 0;
        status->port_suspend = portsc & SUSPEND ? 1 : 0;
        status->port_over_current = 0;
        status->port_reset = portsc & PR ? 1 : 0;
        status->reserved1 = 0;
        status->port_power = 1;
        status->port_low_speed = 0;
        status->port_high_speed = 1;
        status->port_test_mode = 0;
        status->port_indicator_ctrl = 0;
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
        uint32_t portindex = cmd->index - 1;
        uint32_t portsc = info->opregs->portsc[portindex];
        int32_t  lowspeed = 0;
        if (cmd->value == PORT_RESET) {
            if ((portsc & LS) != 0x400) {
                portsc = (portsc & 0xFFFFFFF5) | PR;
            } else {
                lowspeed = 1;
            }
        } else if (cmd->value == PORT_SUSPEND) {
            portsc = (portsc & 0xFFFFFFF5) | SUSPEND;
        } else if (cmd->value == PORT_POWER) {
            portsc = (portsc & 0xFFFFFFF5);
        }
        info->opregs->portsc[portindex] = portsc;
        if (cmd->value == PORT_RESET) {
            if (!lowspeed) {
                /* delay */
                sleep(20);
                /* stop reset process */
                portsc &= 0xFFFFFFF5 & ~PR;
                info->opregs->portsc[portindex] = (portsc |= PED);
                while (info->opregs->portsc[portindex] & PR);
            }
            /* if high speed device, PED goes to 1. */
            if (lowspeed || !(info->opregs->portsc[portindex] & PED)) {
                /* not high speed */
                /* printk("not high speed! %d %x\n", lowspeed,
                       info->opregs->portsc[portindex]); */
                portsc = info->opregs->portsc[portindex];
                portsc = (portsc & 0xFFFFFFF5) | PO; /* release ownership */
                info->opregs->portsc[portindex] = portsc;
                /* wait until disconnected */
                while (info->opregs->portsc[portindex] & CCS);
            }
        }
        return 0;
    } else {
        return -EIO;
    }
}

/* ================================================================= */
/*                            Transactions                           */
/* ================================================================= */

int32_t ehci_transfer(info_t *info, int32_t count, transaction_t *trans) {
    qtd_t *qtds = kmalloc(sizeof(qtd_t)*count);
    qh_t *qh = kmalloc(sizeof(qh_t));
    int32_t i, j, rem;
    uint32_t next_page, actual = 0;
    uint8_t status;
    usb_device_t *usbdev = trans[0].usbdev;

    /* initialize qtds */
    for (i = 0; i < count; i++) {
        /* initialize next link */
        if (i < count - 1) {
            qtds[i].nextT       = 0;
            qtds[i].Zero1       = 0;
            qtds[i].nextQTDP    = TO_PHYS(&qtds[i+1])>>5;
            qtds[i].AltNextT    = 0;
            qtds[i].Zero2       = 0;
            qtds[i].AltNextQTDP = TO_PHYS(&qtds[i+1])>>5;
        } else {
            qtds[i].nextT       = 1;
            qtds[i].Zero1       = 0;
            qtds[i].nextQTDP    = 0;
            qtds[i].AltNextT    = 1;
            qtds[i].Zero2       = 0;
            qtds[i].AltNextQTDP = 0;
        }
        /* initialize token */
        qtds[i].Status      = 0x80;
        switch (trans[i].tokpid) {
            case PID_SETUP:
                qtds[i].PIDCode = 2;
                break;
            case PID_IN:
                qtds[i].PIDCode = 1;
                break;
            case PID_OUT:
                qtds[i].PIDCode = 0;
                break;
        }
        qtds[i].Cerr        = 3;
        qtds[i].C_Page      = 0;
        qtds[i].ioc         = 0;
        qtds[i].TotalBytes  = trans[i].pktsize;
        /*printk("tot: %x\n", qtds[i].TotalBytes);*/
        qtds[i].dt          = trans[i].datapid == PID_DATA1;
        /*printk("dt: %d\n", qtds[i].dt);*/
        /* initialize buffer ptr list */
        /*printk("off: %x\n", TO_PHYS(trans[i].databuf));*/
        qtds[i].CurOffset   = TO_PHYS(trans[i].databuf)&0xFFF;
        qtds[i].BufPtr0     = TO_PHYS(trans[i].databuf)>>12;
        next_page = (((uint32_t)trans[i].databuf) & 0xFFFFF000) + 0x1000;
        rem = trans[i].pktsize-(next_page-((uint32_t)trans[i].databuf));
        qtds[i].Reserved1   = 0;
        if (rem > 0) {
            qtds[i].BufPtr1 = TO_PHYS(next_page)>>12;
            next_page = next_page + 0x1000;
            rem -= 0x1000;
        } else {
            qtds[i].BufPtr1 = 0;
        }
        qtds[i].Reserved2   = 0;
        if (rem > 0) {
            qtds[i].BufPtr2 = TO_PHYS(next_page)>>12;
            next_page = next_page + 0x1000;
            rem -= 0x1000;
        } else {
            qtds[i].BufPtr2 = 0;
        }
        qtds[i].Reserved3   = 0;
        if (rem > 0) {
            qtds[i].BufPtr3 = TO_PHYS(next_page)>>12;
            next_page = next_page + 0x1000;
            rem -= 0x1000;
        } else {
            qtds[i].BufPtr3 = 0;
        }
        qtds[i].Reserved4   = 0;
        if (rem > 0) {
            qtds[i].BufPtr4 = TO_PHYS(next_page)>>12;
            next_page = next_page + 0x1000;
            rem -= 0x1000;
        } else {
            qtds[i].BufPtr4 = 0;
        }
        qtds[i].ExtBufPtr0  = 0;
        qtds[i].ExtBufPtr1  = 0;
        qtds[i].ExtBufPtr2  = 0;
        qtds[i].ExtBufPtr3  = 0;
        qtds[i].ExtBufPtr4  = 0;
    }

    /* initialize QH */
    qh->T           = 0;
    qh->Typ         = TYP_QH;
    qh->Zero1       = 0;
    qh->QHHLP       = 0;

    qh->DevAddr     = trans[0].addr;
    qh->I           = 0;
    qh->EndPt       = trans[0].endpoint;
    qh->EPS         = (usbdev->highspeed<<1) | usbdev->lowspeed;
    qh->dtc         = 1;
    qh->H           = 0;
    qh->MaxPacket   = trans[0].maxpacket;
    qh->C           = qh->EPS != 2 && trans[0].tokpid == PID_SETUP;
    qh->RL          = 3;

    qh->uFrSmask    = 0;
    qh->uFrCmask    = 0;
    qh->HubAddr     = usbdev->hubdev->addr;
    qh->PortNum     = usbdev->port;
    qh->Mult        = 1;

    qh->Zero2       = 0;
    qh->curQTDP     = 0;

    qh->nextT       = 0;
    qh->Zero3       = 0;
    qh->nextQTDP    = TO_PHYS(&qtds[0])>>5;

    qh->AltNextT    = 0;
    qh->NakCnt      = 0;
    qh->AltNextQTDP = TO_PHYS(&qtds[0])>>5;

    qh->Status      = 0;
    qh->PIDCode     = 0;
    qh->Cerr        = 0;
    qh->C_Page      = 0;
    qh->ioc         = 0;
    qh->TotalBytes  = 0;
    qh->dt          = 0;

    qh->CurOffset   = 0;
    qh->BufPtr0     = 0;

    qh->CProgMask   = 0;
    qh->Reserved1   = 0;
    qh->BufPtr1     = 0;

    qh->FrameTag    = 0;
    qh->SBytes      = 0;
    qh->BufPtr2     = 0;

    qh->Reserved2   = 0;
    qh->BufPtr3     = 0;

    qh->Reserved3   = 0;
    qh->BufPtr4     = 0;

    qh->ExtBufPtr0  = 0;
    qh->ExtBufPtr1  = 0;
    qh->ExtBufPtr2  = 0;
    qh->ExtBufPtr3  = 0;
    qh->ExtBufPtr4  = 0;

    /* insert QH into linked list */
    qh->QHHLP = info->qhfirst->QHHLP;
    info->qhfirst->QHHLP = TO_PHYS(qh)>>5;

    /* wait for transfer to finish */
#if 0
    printk("transfer begin.\n");
    for (j = 0; j < sizeof(qh_t)/4; j++) {
        printk("%x ", ((int *) qh)[j]);
    }
    printk("\n");
#endif
/*
    for (i = 0; i < count; i++) {
        printk("%d: ", );
    }
*/
    for (i = 0; i < count; i++) {
        /*printk("i: %d of %d - %x\n", i, count, info->opregs->usbsts);*/
        while ((status = qtds[i].Status) & 0x80);
        if (usbdev->highspeed) {
            status &= 0xFE;
        }
        if (status) {
            printk("EHCI error happened %d %x %x!\n", i, status,
                   qh->Cerr);
            break;
        }
        if (trans[i].calc)
            actual += trans[i].pktsize - qtds[i].TotalBytes;
    }

#if 0
    for (j = 0; j < sizeof(qtd_t)/4; j++) {
        printk("%x ", ((int *) &qtds[0])[j]);
    }
    printk("\n");
#endif

    /* remove QH from the linked list */
    info->qhfirst->QHHLP = qh->QHHLP;
    info->opregs->usbcmd |= INTASYNCENABLE;
    while (!(info->opregs->usbsts & INTASYNC));
    info->opregs->usbsts = INTASYNC;

    /* deallocate QH and qTDs*/
    kfree(qh);
    kfree(qtds);

    /* done */
    if (status)
        return -EIO;
    else
        return actual;
}

/* ================================================================= */
/*                       UHCI/OHCI Companionship                     */
/* ================================================================= */

int32_t ehci_set_companion(device_t *dev, pci_config_t *pci_config) {
    companion_t *comp, *prev;
    uint32_t bus, devno, func, i;
    device_t *pcidev = dev->parent_bus->ctl;
    class_t cls;

    /* already in the list? */
    comp = compfirst;
    prev = NULL;
    while (comp) {
        if (comp->dev == dev) {
            return -1;
        }
        prev = comp;
        comp = comp->next;
    }

    /* get PCI parameters */
    bus = pci_config->bus;
    devno = pci_config->devno;
    func = pci_config->func;

    /* is there a companion EHCI? */
    for (i = func+1; i <= 7; i++) {
        if (read_conf16(pcidev, bus, devno, i, PCI_REG_VENDOR_ID) != 0xFFFF) {
            /* device exists */
            cls.bus    = BUS_PCI;
            cls.base   = read_conf8(pcidev, bus, devno, i, PCI_REG_CLASS);
            cls.sub    = read_conf8(pcidev, bus, devno, i, PCI_REG_SUBCLASS);
            cls.progif = read_conf8(pcidev, bus, devno, i, PCI_REG_PROG_IF);
            if (cls.bus    == ehci_driver.cls[0].bus  &&
                cls.base   == ehci_driver.cls[0].base &&
                cls.sub    == ehci_driver.cls[0].sub  &&
                cls.progif == ehci_driver.cls[0].progif) {
                break;
            }
        }
    }
    if (i == 8) {
        /* no EHCI companion controller found */
        return -1;
    }

    /* add to list */
    comp = kmalloc(sizeof(companion_t));
    comp->next = NULL;
    comp->dev = dev;
    comp->pci_config = *pci_config;
    if (prev == NULL) {
        compfirst = comp;
    } else {
        prev->next = comp;
    }

    /* done */
    return 0;
}

void init_companions(uint32_t bus, uint32_t devno, uint32_t func) {
    companion_t *comp = compfirst;
    while (comp) {
        if (comp->pci_config.bus == bus &&
            comp->pci_config.devno == devno &&
            comp->pci_config.func < func) {
            /* This is one of the companion controllers that postponed
             * their initialization procedure to favor EHCI.
             */
            comp->dev->driver->probe(comp->dev, &comp->pci_config);
        }
        comp = comp->next;
    }
}

/* ================================================================= */
/*                             Interface                             */
/* ================================================================= */

uint32_t ehci_probe(device_t *dev, void *config) {

    resource_t *list = dev->resources.list;
    int32_t i;
    uint32_t eecp, usblegsup;
    device_t *pcidev = dev->parent_bus->ctl;
    uint32_t busno = ((pci_config_t *) config)->bus;
    uint32_t devno = ((pci_config_t *) config)->devno;
    uint32_t func  = ((pci_config_t *) config)->func;

    /* create info_t structure: */
    info_t *info = (info_t *) kmalloc(sizeof(info_t));
    dev->drvreg = (uint32_t) info;
    if (info == NULL)
        return ENOMEM; /* i am sorry :D */

    /* inform user of our progress */
    printk("%aUSB%a: ", 0x0A, 0x0F);
    printk("Enhanced host controller interface (EHCI) on PCI.\n");

    /* get BARs and irqnum */
#if 0
    for (i = 0; i < 7; i++) {
        printk("Type: %d, base: %x\n", list[i].type,
               info->iobase = list[i].data.port.base);
    }
#endif
    info->iotype = list[1].type; /* memory */
    info->iobase = list[1].data.port.base;
    info->irqnum = list[0].data.irq.number;

    /* get size of register set */
    /* length of capability register set is found in the first
     * 8-bit register of capability registers:
     */
    info->caplen = pmem_readb(info->iobase);
    /* max number of ports is 15; operational reg set = 0x44+no_of_ports*4
     * so opreg set max size = 0x44+0x3C = 0x80.
     * according to specs page 19, info->iobase is 0x100-byte aligned.
     * If caplen is <= 0x80, the register set can never cross page
     * boundaries. caplen is usually 0x20 byes only, so we can safely
     * assume that it will never reach 0x80 to simplify the code.
     */
    info->capregs = kmalloc(PAGE_SIZE);
    arch_set_page(NULL, info->capregs, info->iobase & PAGE_BASE_MASK);
    /* info->iobase might not be 4K-aligned */
    info->capregs = (capregs_t *)(((uint32_t)info->capregs)+
                                   (info->iobase&(PAGE_SIZE-1)));
    /* now set opregs */
    info->opregs = (opregs_t *)(((uint32_t)info->capregs) + info->caplen);
    /* get number of ports */
    info->portcount = info->capregs->hcsparams & 0x000F;

    /* disable USB legacy support */
    eecp = (info->capregs->hccparams & EECP)>>8;
    while (eecp) {
        usblegsup = read_conf32(pcidev, busno, devno, func, eecp);
        if ((usblegsup & 0xFF) == 0x01) {
            /* legacy support */
            usblegsup |= (1<<24);
            write_conf32(pcidev, busno, devno, func, eecp, usblegsup);
            while (1) {
                usblegsup = read_conf32(pcidev, busno, devno, func, eecp);
                if ((usblegsup & (1<<24)) && !(usblegsup & (1<<16))) {
                    break;
                }
            }
            write_conf32(pcidev, busno, devno, func, eecp+4, 0);
            break;
        } else {
            /* get next extended capability pointer */
            eecp = (usblegsup>>8) & 0xFF;
        }
    }

    /* allocate qhfirst */
    info->qhfirst = kmalloc(sizeof(qh_t));

    /* initialize qhfirst */
    info->qhfirst->T           = 0;
    info->qhfirst->Typ         = TYP_QH;
    info->qhfirst->Zero1       = 0;
    info->qhfirst->QHHLP       = TO_PHYS(info->qhfirst)>>5;

    info->qhfirst->DevAddr     = 0;
    info->qhfirst->I           = 0;
    info->qhfirst->EndPt       = 0;
    info->qhfirst->EPS         = 0;
    info->qhfirst->dtc         = 0;
    info->qhfirst->H           = 1;
    info->qhfirst->MaxPacket   = 0;
    info->qhfirst->C           = 0;
    info->qhfirst->RL          = 0;

    info->qhfirst->uFrSmask    = 0;
    info->qhfirst->uFrCmask    = 0;
    info->qhfirst->HubAddr     = 0;
    info->qhfirst->PortNum     = 0;
    info->qhfirst->Mult        = 1;

    info->qhfirst->Zero2       = 0;
    info->qhfirst->curQTDP     = 0;

    info->qhfirst->nextT       = 1;
    info->qhfirst->Zero3       = 0;
    info->qhfirst->nextQTDP    = 0;

    info->qhfirst->AltNextT    = 1;
    info->qhfirst->NakCnt      = 0;
    info->qhfirst->AltNextQTDP = 0;

    info->qhfirst->Status      = 0;
    info->qhfirst->PIDCode     = 0;
    info->qhfirst->Cerr        = 0;
    info->qhfirst->C_Page      = 0;
    info->qhfirst->ioc         = 0;
    info->qhfirst->TotalBytes  = 0;
    info->qhfirst->dt          = 0;

    info->qhfirst->CurOffset   = 0;
    info->qhfirst->BufPtr0     = 0;

    info->qhfirst->CProgMask   = 0;
    info->qhfirst->Reserved1   = 0;
    info->qhfirst->BufPtr1     = 0;

    info->qhfirst->FrameTag    = 0;
    info->qhfirst->SBytes      = 0;
    info->qhfirst->BufPtr2     = 0;

    info->qhfirst->Reserved2   = 0;
    info->qhfirst->BufPtr3     = 0;

    info->qhfirst->Reserved3   = 0;
    info->qhfirst->BufPtr4     = 0;

    info->qhfirst->ExtBufPtr0  = 0;
    info->qhfirst->ExtBufPtr1  = 0;
    info->qhfirst->ExtBufPtr2  = 0;
    info->qhfirst->ExtBufPtr3  = 0;
    info->qhfirst->ExtBufPtr4  = 0;

    /* stop the host controller if running */
    info->opregs->usbcmd = INTTHRESHOLD8; /* RC = 0 */
    while (!(info->opregs->usbsts & HCHALTED));

    /* reset the controller */
    info->opregs->usbcmd = INTTHRESHOLD8 | HCRESET;
    while (info->opregs->usbcmd & HCRESET);

    /* initialize registers */
    info->opregs->usbintr = 0;
    info->opregs->frindex = 0;
    if (info->capregs->hccparams & ADDRESS64)
        info->opregs->ctrldssegment = 0;
    info->opregs->periodiclistbase = 0;
    info->opregs->asynclistaddr = TO_PHYS(info->qhfirst);
    info->opregs->configflag = 1;

    /* enable asynchronous schedule and run the controller */
    info->opregs->usbcmd = INTTHRESHOLD8 | ASYNCENABLE | RS;
#if 0
    while(1) {
        printk("FRINDEX: %x\n", info->opregs->frindex);
    }
#endif

    /* connect power to ports */
    for (i = 0; i < info->portcount; i++)
        info->opregs->portsc[i] = PP;
    sleep(30);

    /* register our EHCI at the USB core system of Quafios */
    usb_reg_hci(dev);

    /* now initialize companion controllers */
    init_companions(busno, devno, func);

    /* done */
    return ESUCCESS;
}

uint32_t ehci_read(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    return ESUCCESS;
}

uint32_t ehci_write(device_t *dev, uint64_t off, uint32_t size,char *buff) {
    return ESUCCESS;
}

uint32_t ehci_ioctl(device_t *dev, uint32_t cmd, void *data) {
    info_t *info = (info_t *) dev->drvreg;
    if (cmd == USB_CMD_HUBCTRL) {
        /* root hub */
        return ehci_root_hub_ctrl_msg(info, data);
    } else {
        return ehci_transfer(info, cmd, data);
    }
}

uint32_t ehci_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
