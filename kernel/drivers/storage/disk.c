/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Abstract Disk Management Layer.                  | |
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
#include <sys/bootinfo.h>
#include <sys/scheduler.h>
#include <storage/disk.h>

disk_t *disks = NULL;

disk_t *get_disk_by_name(char *name) {
    disk_t *disk = disks;
    while (disk && strcmp(disk->name, name))
        disk = disk->next;
    return disk;
}

disk_t *get_disk_by_devid(int32_t devid) {
    disk_t *disk = disks;
    while (disk && disk->dev->devid != devid)
        disk = disk->next;
    return disk;
}

void add_disk(disk_t *disk, char *prefix, int partition) {
    int32_t i;
    /* allocate name */
    disk->name = kmalloc(100);
    strcpy(disk->name, prefix);
    for(i = 0; i < 26; i++) {
        if (partition) {
            disk->name[strlen(prefix)+0] = i+'a';
            disk->name[strlen(prefix)+1] = 0;
        } else {
            itoa(i, &disk->name[strlen(prefix)], 10);
        }
        if (!get_disk_by_name(disk->name))
            break;
    }
    if (i == 26)
        return;
    /* register at devfs */
    devfs_reg(disk->name, disk->dev->devid);
    /* add to linked list */
    disk->next = disks;
    disks = disk;
    /* scan disk for partitions */
    partprobe(disk->dev->devid);
}
