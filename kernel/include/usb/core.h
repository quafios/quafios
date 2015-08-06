/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> USB core header.                                 | |
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

#ifndef USB_CORE_H
#define USB_CORE_H

/* USB protocol data structures */
typedef struct request {
    unsigned char bmRequestType;
#define GET_STATUS         0
#define CLEAR_FEATURE      1
#define SET_FEATURE        3
#define SET_ADDRESS        5
#define GET_DESCRIPTOR     6
#define SET_DESCRIPTOR     7
#define GET_CONFIGURATION  8
#define SET_CONFIGURATION  9
#define GET_INTERFACE      10
#define SET_INTERFACE      11
#define SYNCH_FRAME        12
    unsigned char bRequest;
    unsigned int  wValue;
    unsigned int  wIndex;
    unsigned int  wLength;
} request_t;

#define DEVICE_DESCRIPTOR        1
#define CONFIG_DESCRIPTOR        2
#define STRING_DESCRIPTOR        3
#define INTERFACE_DESCRIPTOR     4
#define ENDPOINT_DESCRIPTOR      5

typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdUSB;
    uint8_t  bDeviceClass;
    uint8_t  bDeviceSubClass;
    uint8_t  bDeviceProtocol;
    uint8_t  bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t  iManufacturer;
    uint8_t  iProduct;
    uint8_t  iSerialNumber;
    uint8_t  bNumConfigurations;
} device_desc_t;

typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
    uint8_t  bNumInterfaces;
    uint8_t  bConfigurationValue;
    uint8_t  iConfiguration;
    uint8_t  bmAttributes;
    uint8_t  maxPower;
} conf_desc_t;

typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bInterfaceNumber;
    uint8_t  bAlternateSetting;
    uint8_t  bNumEndpoints;
    uint8_t  bInterfaceClass;
    uint8_t  bInterfaceSubClass;
    uint8_t  bInterfacePorotocol;
    uint8_t  iInterface;
} interface_desc_t;

typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bEndpointAddress;
    uint8_t  bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t  bInterval;
} endpoint_desc_t;

typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bString[30];
} string_desc_t;

/* features */
#define ENDPOINT_HALT   0

/* usb hierarchy */
typedef struct usb_hci {
    struct usb_hci *next;
    device_t *dev;
    int32_t addr_pool[128];
    int32_t last_addr;
} usb_hci_t;

typedef struct usb_device {
    usb_hci_t         *hci;
    int32_t            addr;
    struct usb_device *hubdev;
    int32_t            port;
    int32_t            lowspeed;
    int32_t            highspeed;
    int32_t            maxpacket0;
    device_desc_t     *dev_desc;
    conf_desc_t       *conf_desc;
    interface_desc_t  *if_desc[32];
    endpoint_desc_t   *endpt_desc[16];
    int32_t            endpt_toggle[16];
    char              *str_manufact;
    char              *str_product;
    char              *str_serial;
} usb_device_t;

typedef struct usb_interface {
    device_t *dev;
    usb_device_t *usbdev;
    int32_t if_num;
    int32_t first_ep;
} usb_interface_t;

/* transactions and commands */
typedef struct transaction {
#define PID_SETUP       0x2D
#define PID_IN          0x69
#define PID_OUT         0xE1
    int32_t tokpid;
    int32_t addr;
    int32_t endpoint;
#define PID_DATA0       0xC3
#define PID_DATA1       0x4B
    int32_t datapid;
    char *databuf;
    int32_t pktsize;
    int32_t lowspeed;
    int32_t highspeed;
    uint32_t maxpacket;
    usb_device_t *usbdev;
    int32_t calc;
} transaction_t;

#define USB_CMD_HUBCTRL         0
#define USB_CMD_DO_TRANSFER     1

typedef struct usb_cmd {
    usb_device_t *dev;
    uint32_t      pipe;
    uint8_t       request;
    uint8_t       requesttype;
    uint16_t      value;
    uint16_t      index;
    void         *data;
    uint16_t      size;
    int           timeout;
} usb_cmd_t;

/* prototypes */
int32_t usb_alloc_addr(usb_hci_t *hci);
void usb_release_addr(usb_hci_t *hci, int32_t addr);
void usb_reg_hci(device_t *dev);
int32_t usb_control_msg(usb_device_t *dev,
                        uint32_t      pipe,
                        uint8_t       request,
                        uint8_t       requesttype,
                        uint16_t      value,
                        uint16_t      index,
                        void         *data,
                        uint16_t      size,
                        int           timeout);
int32_t usb_bulk_msg(usb_device_t *usb_dev,
                     uint32_t      pipe,
                     void         *data,
                     int32_t       len,
                     int32_t      *actual_length,
                     int32_t       timeout);
int32_t usb_enum_bus(usb_interface_t *hubif, int32_t port);

#endif
