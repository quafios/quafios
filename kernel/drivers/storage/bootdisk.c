/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Boot disk detection.                             | |
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
#include <sys/device.h>
#include <storage/disk.h>
#include <fs/diskfs.h>
#include <sys/mm.h>
#include <sys/bootinfo.h>

extern bootinfo_t *bootinfo;

int diskfs_getuuid(device_t *dev, uint8_t *uuid) {
    diskfs_sb_t *sb;
    int32_t i;
    int32_t ret = -1;
    sb = kmalloc(512);
    if (dev_read(dev, (int64_t) 2*512, 512, (char *) sb))
        return ret;
    if (sb->magic == QUAFS_MAGIC) {
        /* diskfs */
        for (i = 0; i < 17; i++)
            uuid[i] = sb->uuid[i];
        ret = 0;
    }
    kfree(sb);
    return ret;
}

int32_t detect_bootdisk() {
    extern linkedlist devices;
    device_t *dev = (device_t *) devices.first;
    uint8_t uuid[17];
    int32_t i;
    while (dev) {
        disk_t *disk = (disk_t *) get_disk_by_devid(dev->devid);
        if ((disk && !disk->partitioned) ||
            (dev->cls.bus==BUS_DISK && dev->cls.base==BASE_DISK_PARTITION)) {
            /* try diskfs */
            if (!diskfs_getuuid(dev, uuid)) {
                for (i = 0; i < 16 && uuid[i] == bootinfo->uuid[i]; i++);
                if (i == 16) {
                    return dev->devid;
                }
            }
        }
        dev = dev->next;
    }
    return -1;
}
