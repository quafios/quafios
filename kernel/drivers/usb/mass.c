/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> USB Subsystem: Mass Storage Device Driver        | |
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
#include <scsi/scsi.h>

/* Prototypes: */
uint32_t usbmass_probe(device_t *, void *);
uint32_t usbmass_read (device_t *, uint64_t, uint32_t, char *);
uint32_t usbmass_write(device_t *, uint64_t, uint32_t, char *);
uint32_t usbmass_ioctl(device_t *, uint32_t, void *);
uint32_t usbmass_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_USB, BASE_USB_MASS_STORAGE,
        SUB_USB_MASS_STORAGE_SCSI, IF_ANY}
};

/* driver_t structure that identifies this driver: */
driver_t usbmass_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "usbmass",
    /* probe:     */ usbmass_probe,
    /* read:      */ usbmass_read,
    /* write:     */ usbmass_write,
    /* ioctl:     */ usbmass_ioctl,
    /* irq:       */ usbmass_irq
};

/* command block wrapper */
typedef struct cbw {
    uint32_t dCBWSignature;
    uint32_t dCBWTag;
    uint32_t dCBWDataTransferLength;
    uint8_t  bmCBWFlags;
    uint8_t  bCBWLUN;
    uint8_t  bCBWCBLength;
    uint8_t  CBWCB[16];
} cbw_t;

/* command status wrapper */
typedef struct csw {
    uint32_t dCSWSignature;
    uint32_t dCSWTag;
    uint32_t dCSWDataResidue;
    uint8_t  bCSWStatus;
} csw_t;

/* info_t structure */
typedef struct info {
    usb_interface_t *usbif;
    usb_device_t *usbdev;
} info_t;

/* ================================================================= */
/*                         Hardware Interface                        */
/* ================================================================= */

static int32_t issue_transfer(info_t *info, int32_t lun, uint8_t *cdb,
                              int32_t cdb_len, char *data,
                              int32_t data_len, int32_t is_write) {
    cbw_t cbw;
    csw_t csw;
    int32_t i, in_pipe, out_pipe, retval, actlen;

    /* determine pipes */
    in_pipe = info->usbif->first_ep | 0x80; /* 0x81 */
    out_pipe = info->usbif->first_ep+1; /* 0x20 */

    /* initialize cbw */
    cbw.dCBWSignature = 0x43425355;
    cbw.dCBWTag = 0x1ABCDEF1;
    cbw.dCBWDataTransferLength = data_len;
    cbw.bmCBWFlags = (!is_write)<<7;
    cbw.bCBWLUN = lun;
    cbw.bCBWCBLength = cdb_len;
    for (i = 0; i < cdb_len; i++)
        cbw.CBWCB[i] = cdb[i];

    /* send cbw */
    retval = usb_bulk_msg(info->usbdev, out_pipe, &cbw, 31, &actlen, 2000);
    if (retval)
        return -retval;

    /* transfer data */
    retval = usb_bulk_msg(info->usbdev,
                          is_write ? out_pipe : in_pipe,
                          data, data_len, &actlen, 2000);
    if (retval) {
        /* clear stall condition */
        retval = usb_control_msg(info->usbdev, 0, CLEAR_FEATURE, 0x02,
                                 ENDPOINT_HALT,
                                 is_write ? out_pipe : in_pipe,
                                 NULL, 0, 2000);
        info->usbdev->endpt_toggle[(is_write?out_pipe:in_pipe)&15] = 0;

        /* read status */
        retval = usb_bulk_msg(info->usbdev, in_pipe, &csw, 13, &actlen, 2000);
        if (retval)
            return -retval;

        /* retry */
        retval = usb_bulk_msg(info->usbdev, out_pipe, &cbw, 31, &actlen, 2000);
        if (retval)
            return -retval;

        /* read data */
        retval = usb_bulk_msg(info->usbdev,
                              is_write ? out_pipe : in_pipe,
                              data, data_len, &actlen, 2000);
        if (retval)
            return -retval;
    }

# if 0
    for (i = 0; i < data_len; i++) {
        printk("%c", data[i]);
    }
    printk("\n");
#endif

    /* get csw */
    retval = usb_bulk_msg(info->usbdev, in_pipe, &csw, 13, &actlen, 2000);
    if (retval)
        return -retval;

    /* return error code */
    return csw.bCSWStatus;
}

/* ================================================================= */
/*                             Interface                             */
/* ================================================================= */

uint32_t usbmass_probe(device_t *dev, void *config) {

    int32_t retval, i;
    uint8_t max_lun;
    bus_t *scsibus;

    /* create info_t structure: */
    info_t *info = (info_t *) kmalloc(sizeof(info_t));
    dev->drvreg = (uint32_t) info;
    if (info == NULL)
        return ENOMEM; /* i am sorry :D */

    /* store usb interface in info structure */
    info->usbif = (usb_interface_t *) config;
    info->usbdev = info->usbif->usbdev;

    /* make scsi bus */
    dev_mkbus(&scsibus, BUS_SCSI, dev);

    /* send reset command to the device */
    retval = usb_control_msg(info->usbdev, 0, 0xFF, 0x21, 0,
                             info->usbif->if_num, NULL, 0, 2000);
    if (retval < 0) {
        printk("%aUSB Mass Storage Error: reset failed.%a\n", 0x0C, 0x0F);
        return retval;
    }

    /* get max LUN */
    retval = usb_control_msg(info->usbdev, 0, 0xFE, 0xA1, 0,
                             info->usbif->if_num, &max_lun, 1, 2000);
    if (retval < 0) {
        printk("%aUSB Mass Storage Error: reset failed.%a\n", 0x0C, 0x0F);
        while(1);
        return retval;
    }

    /* add drivers for attached units */
    for (i = 0; i <= max_lun; i++) {
        device_t *subdev;
        class_t cls;
        reslist_t reslist = {0, NULL};
        scsi_config_t scsi_config;
        cls.bus    = BUS_SCSI;
        cls.base   = BASE_SCSI_DISK;
        cls.sub    = SUB_SCSI_DISK_GENERIC;
        cls.progif = IF_ANY;
        scsi_config.ctrlr = dev;
        scsi_config.lun   = i;
        dev_add(&subdev, scsibus, cls, reslist, &scsi_config);
    }

    /* done */
    return ESUCCESS;
}

uint32_t usbmass_read(device_t *dev, uint64_t off, uint32_t size, char *buff){
    return ESUCCESS;
}

uint32_t usbmass_write(device_t *dev, uint64_t off, uint32_t size, char *buff){
    return ESUCCESS;
}

uint32_t usbmass_ioctl(device_t *dev, uint32_t cmd, void *data) {
    info_t *info = (info_t *) dev->drvreg;
    scsi_cmd_t *scsi_cmd = (scsi_cmd_t *) data;
    return issue_transfer(info, scsi_cmd->lun, scsi_cmd->cdb,
                          scsi_cmd->cdb_len, scsi_cmd->data,
                          scsi_cmd->data_len, scsi_cmd->is_write);
}

uint32_t usbmass_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
