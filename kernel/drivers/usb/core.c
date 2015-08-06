/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> USB Subsystem: Core                              | |
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
#include <usb/core.h>
#include <usb/hub.h>

/* ================================================================= */
/*                         Classification                            */
/* ================================================================= */

typedef struct usb_class {
    uint8_t base;
    char *name;
} usb_class_t;

usb_class_t usb_classes[] = {
    {0x00,"USB Composite Device"},
    {0x01,"USB Audio Device"},
    {0x02,"USB Communications Device"},
    {0x03,"USB HID Device"},
    {0x05,"USB Physical Device"},
    {0x06,"USB Image Device"},
    {0x07,"USB Printer Device"},
    {0x08,"USB Mass Storage Device"},
    {0x09,"USB Hub"},
    {0x0A,"USB CDC-Data Device"},
    {0x0B,"USB Smart Card"},
    {0x0D,"USB Content Security Device"},
    {0x0E,"USB Video Device"},
    {0x0F,"USB Personal Healthcare Device"},
    {0x10,"USB Audio/Video Device"},
    {0x11,"USB Billboard Device"},
    {0xDC,"USB Diagnostic Device"},
    {0xE0,"USB Wireless Controller"},
    {0xEF,"USB Miscellaneous Device"},
    {0xFE,"USB Application-Specific Device"},
    {0xFF,"USB Vendor-Specific Device"}
};

static char *usb_base_to_name(uint8_t base) {
    int32_t i;
    for (i = 0; i < sizeof(usb_classes)/sizeof(usb_classes[0]); i++) {
        if (base == usb_classes[i].base) {
            return usb_classes[i].name;
        }
    }
    return "USB Unknown Device";
}

/* ================================================================= */
/*                           HCI control                             */
/* ================================================================= */

usb_hci_t *head = NULL;

int32_t usb_alloc_addr(usb_hci_t *hci) {
    int32_t i, addr;
    /* allocate an address for a new usb device */
    for (i = 0; i < 128; i++) {
        if (!hci->addr_pool[addr = (hci->last_addr+i)&0x7F])
            break;
    }
    if (i == 128)
        return -1; /* no address can be allocated */
    hci->last_addr = addr + 1;
    hci->addr_pool[addr] = 1;
    return addr;
}

void usb_release_addr(usb_hci_t *hci, int32_t addr) {
    hci->addr_pool[addr] = 0;
}

void usb_reg_hci(device_t *dev) {
    int i;
    usb_hci_t *hci;
    usb_device_t *roothub;
    usb_interface_t *roothubif;
    bus_t *usbbus;
    class_t cls = {BUS_USB, BASE_USB_HUB, SUB_USB_HUB, IF_USB_HUB_ROOTHUB};
    reslist_t reslist = {0, NULL};
    /* allocate usb_hci structure */
    hci = kmalloc(sizeof(usb_hci_t));
    /* initialize hci structure */
    hci->dev  = dev;
    hci->addr_pool[0] = 1;
    for (i = 1; i < 128; i++)
        hci->addr_pool[i] = 0;
    hci->last_addr = 1;
    /* add the hci to list of registered HCIs */
    hci->next = head;
    head = hci;
    /* create a bus for the HCI */
    dev_mkbus(&usbbus, BUS_USB, dev);
    /* create roothub usb_device_t structure */
    roothub       = kmalloc(sizeof(usb_device_t));
    roothub->hci  = hci;
    roothub->addr = usb_alloc_addr(hci);
    /* create roothub interface structure */
    roothubif     = kmalloc(sizeof(usb_interface_t));
    roothubif->usbdev = roothub;
    roothubif->if_num = 0;
    roothubif->first_ep = 16;
    /* add root hub to device hierarchy */
    dev_add(&roothubif->dev, usbbus, cls, reslist, roothubif);
}

/* ================================================================= */
/*                         Transfer control                          */
/* ================================================================= */

int32_t usb_control_msg(usb_device_t *dev,
                        uint32_t      pipe,
                        uint8_t       request,
                        uint8_t       requesttype,
                        uint16_t      value,
                        uint16_t      index,
                        void         *data,
                        uint16_t      size,
                        int32_t       timeout) {
    /* handle usb control transfers */
    usb_cmd_t usb_cmd;
    char setup_data[8], status_data[2] = {0};
    int32_t packet_size;
    int32_t direction; /* 1: read from device (IN), 0: write (OUT) */
    int32_t no_of_data_packets;
    int32_t i;
    int32_t retval;
    transaction_t *trans;

    /* special case: root hub */
    if (dev->addr == 1) {
        usb_cmd.dev = dev;
        usb_cmd.pipe = pipe;
        usb_cmd.request = request;
        usb_cmd.requesttype = requesttype;
        usb_cmd.value = value;
        usb_cmd.index = index;
        usb_cmd.data = data;
        usb_cmd.size = size;
        usb_cmd.timeout = timeout;
        return dev_ioctl(dev->hci->dev, USB_CMD_HUBCTRL, &usb_cmd);
    }

    /* determine packet size */
    if (pipe == 0) {
        packet_size = dev->maxpacket0;
    } else {
        packet_size = dev->endpt_desc[pipe]->wMaxPacketSize;
    }

    /* determine no of data packets */
    no_of_data_packets = size/packet_size + (size%packet_size?1:0);

    /* determine direction */
    direction = requesttype&0x80 ? 1 : 0;

    /* initialize setup data */
    setup_data[0] = requesttype;
    setup_data[1] = request;
    setup_data[2] = value&0xFF;
    setup_data[3] = value>>8;
    setup_data[4] = index&0xFF;
    setup_data[5] = index>>8;
    setup_data[6] = size&0xFF;
    setup_data[7] = size>>8;

    /* allocate transaction data structures */
    trans = kmalloc(sizeof(transaction_t)*(no_of_data_packets+2));

    /* setup transaction */
    trans[0].tokpid    = PID_SETUP;
    trans[0].addr      = dev->addr;
    trans[0].endpoint  = pipe;
    trans[0].datapid   = PID_DATA0;
    trans[0].databuf   = setup_data;
    trans[0].pktsize   = 8;
    trans[0].lowspeed  = dev->lowspeed;
    trans[0].highspeed = dev->highspeed;
    trans[0].maxpacket = packet_size;
    trans[0].usbdev    = dev;
    trans[0].calc      = 0;

    /* data transactions */
    for (i = 1; i <= no_of_data_packets; i++) {
        trans[i].tokpid    = direction ? PID_IN : PID_OUT;
        trans[i].addr      = dev->addr;
        trans[i].endpoint  = pipe;
        trans[i].datapid   = i%2 ? PID_DATA1 : PID_DATA0;
        trans[i].databuf   = &((char *)data)[(i-1)*packet_size];
        if (size < packet_size) {
            trans[i].pktsize   = size;
            size = 0;
        } else {
            trans[i].pktsize   = packet_size;
            size -= packet_size;
        }
        trans[i].lowspeed  = dev->lowspeed;
        trans[i].highspeed = dev->highspeed;
        trans[i].maxpacket = packet_size;
        trans[i].usbdev    = dev;
        trans[i].calc      = 1;
    }

    /* status transaction */
    trans[i].tokpid    = direction ? PID_OUT : PID_IN;
    trans[i].addr      = dev->addr;
    trans[i].endpoint  = pipe;
    trans[i].datapid   = PID_DATA1;
    trans[i].databuf   = status_data;
    trans[i].pktsize   = 0;
    trans[i].lowspeed  = dev->lowspeed;
    trans[i].highspeed = dev->highspeed;
    trans[i].maxpacket = packet_size;
    trans[i].usbdev    = dev;
    trans[i].calc      = 0;

    /* now let the HCI handle the transfer */
    retval = dev_ioctl(dev->hci->dev, no_of_data_packets+2, trans);

    /* deallocate temporary data */
    kfree(trans);

    /* return error value */
    return retval;
}

int32_t usb_bulk_msg(usb_device_t *usb_dev,
                     uint32_t      pipe,
                     void         *data,
                     int32_t       len,
                     int32_t      *actual_length,
                     int32_t       timeout) {
    int32_t endpt;
    int32_t direction; /* 1: read from device (IN), 0: write (OUT) */
    int32_t packet_size;
    int32_t no_of_data_packets;
    int32_t i;
    int32_t retval;
    transaction_t *trans;

    /* get endpoint info */
    endpt = pipe & 15;
    if (pipe & 0x80) {
        /* in */
        direction = 1;
    } else {
        /* out */
        direction = 0;
    }
    packet_size = usb_dev->endpt_desc[endpt]->wMaxPacketSize;

    /* calculate no of packets and allocate them */
    no_of_data_packets = len/packet_size + (len%packet_size?1:0);
    trans = kmalloc(sizeof(transaction_t)*no_of_data_packets);
    for (i = 0; i < no_of_data_packets; i++) {
        trans[i].tokpid    = direction ? PID_IN : PID_OUT;
        trans[i].addr      = usb_dev->addr;
        trans[i].endpoint  = endpt;
        trans[i].datapid   = usb_dev->endpt_toggle[endpt]?PID_DATA1:PID_DATA0;
        trans[i].databuf   = &((char *)data)[i*packet_size];
        if (len < packet_size) {
            trans[i].pktsize   = len;
            len = 0;
        } else {
            trans[i].pktsize   = packet_size;
            len -= packet_size;
        }
        trans[i].lowspeed  = usb_dev->lowspeed;
        trans[i].highspeed = usb_dev->highspeed;
        trans[i].maxpacket = packet_size;
        trans[i].usbdev    = usb_dev;
        trans[i].calc      = 1;
        usb_dev->endpt_toggle[endpt] = !usb_dev->endpt_toggle[endpt];
    }

    /* now let the HCI handle the transfer */
    retval = dev_ioctl(usb_dev->hci->dev, no_of_data_packets, trans);

    /* deallocate temporary data */
    kfree(trans);

    /* return error value */
    if (retval >= 0) {
        if (actual_length)
            *actual_length = retval;
        return 0;
    } else {
        return retval;
    }
}

/* ================================================================= */
/*                             Enumeration                           */
/* ================================================================= */

int32_t usb_enum_bus(usb_interface_t *hubif, int32_t port) {
    /* enumerate usb bus after hub change event (9.1.2) */
    char *buf = kmalloc(0x100);
    usb_device_t *usbdev = kmalloc(sizeof(usb_device_t));
    usb_device_t *hub = hubif->usbdev;
    int32_t retval, i, addr;
    wPortStatus_t *status = (wPortStatus_t *) &buf[0];
    wPortChange_t *change = (wPortChange_t *) &buf[2];


    /* wait for at least 100ms to allow completion of an insertion
     * process and for power at the device to become stable
     */
    sleep(100);

    /* issue reset command to port (reset enables the port, too) */
    usb_control_msg(hub,0,SET_FEATURE,0x23,PORT_RESET,port,NULL,0,2000);
    sleep(20);

    /* get status again */
    usb_control_msg(hub,
                    0,
                    GET_STATUS,        /* request */
                    0xA3,              /* request type */
                    0,                 /* value */
                    port,              /* port index */
                    buf,
                    4,
                    2000);
    if (!status->port_connection) {
        /* disconnected during reset; clear c_port_connection. */
        usb_control_msg(hub,
                        0,
                        CLEAR_FEATURE,     /* request */
                        0x23,              /* request type */
                        C_PORT_CONNECTION, /* value */
                        port,              /* port index */
                        NULL,
                        0,
                        2000);
        return 0;
    }

    /* the usb device is now in the default state and can draw
     * no more than 100mA.
     */
    usbdev->hci  = hub->hci;     /* the hci to which the hub is attached */
    usbdev->addr = 0;            /* default address */
    usbdev->hubdev = hubif->usbdev; /* hub device structure */
    usbdev->port = port; /* store hub port */
    usbdev->lowspeed = status->port_low_speed; /* device is lowspeed? */
    usbdev->highspeed = status->port_high_speed; /* device is highspeed? */
    usbdev->maxpacket0 = 0x40;   /* max packet size for endpoint 0 */
    usbdev->str_manufact = NULL;
    usbdev->str_product  = NULL;
    usbdev->str_serial   = NULL;

    /* get maximum packet size */
    retval = usb_control_msg(usbdev, 0, GET_DESCRIPTOR, 0x80,
                             DEVICE_DESCRIPTOR<<8, 0, buf, 0x08, 2000);
    usbdev->maxpacket0 = buf[7];

    /* get whole device descriptor */
    usbdev->dev_desc = kmalloc(sizeof(device_desc_t));
    retval = usb_control_msg(usbdev, 0, GET_DESCRIPTOR, 0x80,
                             DEVICE_DESCRIPTOR<<8, 0, usbdev->dev_desc,
                             18, 2000);

    /* get config descr size */
    retval = usb_control_msg(usbdev, 0, GET_DESCRIPTOR, 0x80,
                             CONFIG_DESCRIPTOR<<8, 0, buf,
                             8, 2000);

    /* allocate config descriptor */
    retval = (((uint8_t)buf[3])<<8)|((uint8_t)buf[2]);
    usbdev->conf_desc = kmalloc(retval);

    /* get the whole config descriptor */
    retval = usb_control_msg(usbdev, 0, GET_DESCRIPTOR, 0x80,
                             CONFIG_DESCRIPTOR<<8, 0, usbdev->conf_desc,
                             retval, 2000);

    /* get manufacturer string */
    if (retval = usbdev->dev_desc->iManufacturer) {
        /* get length */
        usb_control_msg(usbdev, 0, GET_DESCRIPTOR, 0x80,
                                (STRING_DESCRIPTOR<<8) | retval, 0, buf,
                                2, 2000);
        /* get full string */
        retval = usb_control_msg(usbdev, 0, GET_DESCRIPTOR, 0x80,
                                (STRING_DESCRIPTOR<<8) | retval, 0, buf,
                                buf[0], 2000);
        if (retval > 0) {
            retval = (buf[0]-2)/2;
            usbdev->str_manufact = kmalloc(retval+1);
            for (i = 0; i < retval; i++)
                usbdev->str_manufact[i] = buf[i*2+2];
            usbdev->str_manufact[i] = 0;
        }
    } else {
        usbdev->str_manufact = kmalloc(3);
        strcpy(usbdev->str_manufact, "<>");
    }

    /* get product string */
    if (retval = usbdev->dev_desc->iProduct) {
        /* get length */
        usb_control_msg(usbdev, 0, GET_DESCRIPTOR, 0x80,
                                (STRING_DESCRIPTOR<<8) | retval, 0, buf,
                                2, 2000);
        /* get full string */
        retval = usb_control_msg(usbdev, 0, GET_DESCRIPTOR, 0x80,
                                (STRING_DESCRIPTOR<<8) | retval, 0, buf,
                                buf[0], 2000);
        if (retval > 0) {
            retval = (buf[0]-2)/2;
            usbdev->str_product = kmalloc(retval+1);
            for (i = 0; i < retval; i++)
                usbdev->str_product[i] = buf[i*2+2];
            usbdev->str_product[i] = 0;
        }
    } else {
        usbdev->str_product = kmalloc(100);
        strcpy(usbdev->str_product,
               usb_base_to_name(usbdev->dev_desc->bDeviceClass));
    }

    /* get serial string */
    if (retval = usbdev->dev_desc->iSerialNumber) {
        /* get length */
        usb_control_msg(usbdev, 0, GET_DESCRIPTOR, 0x80,
                                (STRING_DESCRIPTOR<<8) | retval, 0, buf,
                                2, 2000);
        /* get full string */
        retval = usb_control_msg(usbdev, 0, GET_DESCRIPTOR, 0x80,
                                (STRING_DESCRIPTOR<<8) | retval, 0, buf,
                                buf[0], 2000);
        if (retval > 0) {
            retval = (buf[0]-2)/2;
            usbdev->str_serial = kmalloc(retval+1);
            for (i = 0; i < retval; i++)
                usbdev->str_serial[i] = buf[i*2+2];
            usbdev->str_serial[i] = 0;
        }
    } else {
        usbdev->str_serial = kmalloc(3);
        strcpy(usbdev->str_serial, "<>");
    }

    /* print string descriptors */
    printk("+ ");
    /*printk("%a%s%a: ", 0x0E, usbdev->str_manufact, 0x0F);*/
    printk("%a%s%a", 0x0E, usbdev->str_product,  0x0F);
    /*printk(" (%a%s%a)\n", 0x0E, usbdev->str_serial,   0x0F);*/
    printk("\n");

    /* allocate address for the device */
    addr = usb_alloc_addr(usbdev->hci);

    /* now send SET ADDRESS request */
    retval = usb_control_msg(usbdev,0,SET_ADDRESS,0x00,addr,0,NULL,0,2000);
    usbdev->addr = addr;

    /* send SET CONFIGURATION request */
    retval = usb_control_msg(usbdev,0,SET_CONFIGURATION,0x00,
                             usbdev->conf_desc->bConfigurationValue,
                             0,NULL,0,2000);

    /* release temp buffers */
    kfree(buf);

    /* enumerate the interfaces and endpoints of the device */
    for (i = 0; i < 32; i++)
        usbdev->if_desc[i] = NULL;
    for (i = 0; i < 16; i++) {
        usbdev->endpt_desc[i] = NULL;
        usbdev->endpt_toggle[i] = 0;
    }
    buf = (char *) usbdev->conf_desc;
    i = usbdev->conf_desc->bLength;
    while (i < usbdev->conf_desc->wTotalLength) {
        /* we've ecnountered a new interface */
        interface_desc_t *if_desc = (interface_desc_t *) &buf[i];
        int32_t if_num = if_desc->bInterfaceNumber;
        int32_t j, first_ep = 16;
        class_t cls;
        reslist_t reslist = {0, NULL};
        usb_interface_t *usbif;
        i += if_desc->bLength;
        if (if_desc->bDescriptorType != INTERFACE_DESCRIPTOR)
            continue;
        if (usbdev->if_desc[if_num])
            continue; /* this is an alternate setting, skip... */
        usbdev->if_desc[if_num] = if_desc;
        for (j = 0; j < if_desc->bNumEndpoints; j++) {
            /* endpoint descriptor */
            endpoint_desc_t *ep_desc = (endpoint_desc_t *) &buf[i];
            int ep_num = ep_desc->bEndpointAddress & 0x0F;
            i += ep_desc->bLength;
            usbdev->endpt_desc[ep_num] = ep_desc;
            if (ep_num < first_ep)
                first_ep = ep_num;
        }
        /* attach a device driver for the encountered interface */
        cls.bus    = BUS_USB;
        cls.base   = if_desc->bInterfaceClass;
        cls.sub    = if_desc->bInterfaceSubClass;
        cls.progif = if_desc->bInterfacePorotocol;
        usbif      = kmalloc(sizeof(usb_interface_t));
        usbif->usbdev = usbdev;
        usbif->if_num = if_num;
        usbif->first_ep = first_ep;
        dev_add(&usbif->dev, hubif->dev->child_bus, cls, reslist, usbif);
    }

    /* return the assigned address */
    return usbdev->addr;
}
