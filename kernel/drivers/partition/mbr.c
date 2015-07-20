/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Partition table routines                         | |
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

typedef struct partent {
    uint8_t   status;
    uint8_t   first_chs[3];
    uint8_t   parttype;
    uint8_t   last_chs[3];
    uint32_t  first_lba;
    uint32_t  sectcount;
} __attribute__((packed)) partent_t;

typedef struct mbr {
    uint8_t   bootloader[446];
    partent_t partents[4];
    uint16_t  signature;
} __attribute__((packed)) mbr_t;

int32_t has_mbr(device_t *dev) {

    /* flag to indicate whether MBR exists or not */
    int f = 0;

    /* create buffers */
    mbr_t *buf = kmalloc(512);

    /* read first sector */
    dev_read(dev, 0, 512, (char *) buf);

    /* check mbr signature */
    if (buf->signature == 0xAA55)
        f = 1;

    /* deallocate buffer */
    kfree(buf);

    /* return */
    return f;

}

void mbr_rec(device_t *dev, int32_t id_base, uint64_t off, uint64_t base) {

    /* partition counter */
    int32_t i;

    /* extended boot record? */
    int32_t is_ext = !!base;

    /* create buffer */
    mbr_t *buf = kmalloc(512);

    /* read partition table */
    dev_read(dev, off*512, 512, (char *) buf);

    /* make sure signature is valid */
    if (buf->signature == 0xAA55) {
        /* loop over partition entries */
        for (i = 0; i < 4; i++) {
            /* partition existing? */
            if (buf->partents[i].parttype) {
                /* calc new_id */
                int32_t new_id = id_base+i;
                /* calc new base */
                int64_t new_base = is_ext ? base : buf->partents[i].first_lba;
                /* calc new off */
                int64_t new_off;
                if (is_ext && i == 1) {
                    new_off = buf->partents[i].first_lba + base;
                } else {
                    new_off = buf->partents[i].first_lba + off;
                }
                /* add if primary partition or logical drive (not ebr ptr) */
                if (!is_ext || !i) {
                    device_t *t;
                    class_t cls;
                    reslist_t reslist = {0, NULL};
                    /* setup partition class */
                    cls.bus    = BUS_DISK;
                    cls.base   = BASE_DISK_PARTITION;
                    cls.sub    = new_id;
                    cls.progif = dev->devid;
                    /* print info */
                    printk("Entry %d: %d (size: %d)\n", new_id,
                             (int32_t) new_off,
                             buf->partents[i].sectcount);
                    /* add device */
                    dev_add(&t, dev->child_bus, cls, reslist, &new_off);
                }
                /* if extended partition, go into */
                if (buf->partents[i].parttype == 0x05 ||
                    buf->partents[i].parttype == 0x0F) {
                    mbr_rec(dev, new_id<5?5:new_id, new_off, new_base);
                }
            }
        }
    }

    /* deallocate buffer */
    kfree(buf);
}

void mbr_scan(device_t *dev) {
    mbr_rec(dev, 1, 0, 0);
}
