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
#include <storage/disk.h>

int32_t partprobe(disk_t *disk) {

    int32_t devid = disk->dev->devid;
    bus_t *diskbus;
    device_t *subdev;
    class_t cls;
    reslist_t reslist = {0, NULL};
    device_t *dev = (device_t *) devid_to_dev(devid);

    /* not a device? */
    if (!devid)
        return -EINVAL;

    /* make generic disk bus */
    dev_mkbus(&diskbus, BUS_DISK, dev);

    /* add partitions */
    if (has_mbr(dev)) {
        disk->partitioned = 1;
        mbr_scan(dev);
    }

    /* done */
    return ESUCCESS;

}
