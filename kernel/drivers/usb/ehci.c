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

#define TO_PHYS(addr) (arch_vmpage_getAddr(NULL,(uint32_t)addr)\
                       +(((uint32_t)addr)&0xFFF))

/* info structure */
typedef struct info {
    device_t *dev;
    uint32_t iotype;
    uint32_t iobase;
    uint32_t irqnum;


} info_t;

/* ================================================================= */
/*                           Register I/O                            */
/* ================================================================= */

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
#if 0
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
#endif
        return 0;
    } else if (cmd->requesttype == 0xA3 && cmd->request == GET_STATE) {
        /* GetBusState request */
        return 0;
    } else if (cmd->requesttype == 0xA0 && cmd->request == GET_DESCRIPTOR) {
        /* GetHubDescriptor request */
#if 0
        hub_descriptor_t *hubdesc = (hub_descriptor_t *) cmd->data;
        hubdesc->bDescLength = 9;
        hubdesc->bDescriptorType = HUB_DESCRIPTOR;
        hubdesc->bNbrPorts = 2;
        hubdesc->wHubCharacteristics = 0x09;
        hubdesc->bPwrOn2PwrGood = 1;
        hubdesc->bHubContrCurrent = 100;
        hubdesc->DeviceRemovable_PortPwrCtrlMask[0] = 0;
        hubdesc->DeviceRemovable_PortPwrCtrlMask[1] = 0xFF;
#endif
        return 9;
    } else if (cmd->requesttype == 0xA0 && cmd->request == GET_STATUS) {
        /* GetHubStatus request */
        return 0;
    } else if (cmd->requesttype == 0xA3 && cmd->request == GET_STATUS) {
        /* GetPortStatus request */
#if 0
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
#endif
        return 4;
    } else if (cmd->requesttype == 0x20 && cmd->request == SET_DESCRIPTOR) {
        /* SetHubDescriptor request */
        return 0;
    } else if (cmd->requesttype == 0x20 && cmd->request == SET_FEATURE) {
        /* SetHubFeature request */
        return 0;
    } else if (cmd->requesttype == 0x23 && cmd->request == SET_FEATURE) {
        /* SetPortFeature request */
#if 0
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
#endif
        return 0;
    } else {
        return -EIO;
    }
}

/* ================================================================= */
/*                            Transactions                           */
/* ================================================================= */

int32_t ehci_transfer(info_t *info, int32_t count, transaction_t *trans) {

}

/* ================================================================= */
/*                       UHCI/OHCI Companionship                     */
/* ================================================================= */

void ehci_init_companion(int32_t busno, int32_t devno, int32_t func) {
    
}

/* ================================================================= */
/*                             Interface                             */
/* ================================================================= */

uint32_t ehci_probe(device_t *dev, void *config) {

    resource_t *list = dev->resources.list;
    int i;

    /* create info_t structure: */
    info_t *info = (info_t *) kmalloc(sizeof(info_t));
    dev->drvreg = (uint32_t) info;
    if (info == NULL)
        return ENOMEM; /* i am sorry :D */

    /* inform user of our progress */
    printk("%aUSB%a: ", 0x0A, 0x0F);
    printk("Universal host controller interface (EHCI) on PCI.\n");


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
