/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios udev daemon.                                | |
 *        | |  -> main() procedure.                                | |
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/class.h>
#include <api/fs.h>

#define MAX_DISK        28

struct {
    int devid;
} disks[MAX_DISK] = {0};

int get_free_disk() {
    int i = 0;
    for (i = 0; i < MAX_DISK; i++)
        if (!disks[i].devid)
            return i;
    return -1;
}

int get_disk_by_devid(int devid) {
    int i = 0;
    for (i = 0; i < MAX_DISK; i++)
        if (disks[i].devid == devid)
            return i;
    return -1;

}

int main() {

    FILE *dev = fopen("/sys/dev", "r");
    char alias[80];
    int disk_indx = 0;
    int ramdisk_indx = 0;
    int cdrom_indx = 0;
    char filename[200];

    /* chdir to /dev */
    chdir("/dev");

    /* create all drives */
    while(fscanf(dev, "%s", alias) != EOF) {
        int devid;
        int bus, base, sub, progif;
        fscanf(dev, "%d", &devid);
        fscanf(dev, "%d", &bus);
        fscanf(dev, "%d", &base);
        fscanf(dev, "%d", &sub);
        fscanf(dev, "%d", &progif);
        if (bus == BUS_MEMORY && base == BASE_MEMORY_NODE) {
            if (sub == SUB_MEMORY_NODE_RAMDISK) {
                /* ramdisk */
                strcpy(filename, "ramdisk");
                itoa(ramdisk_indx++, &filename[strlen(filename)], 10);
                mknod(filename, FT_SPECIAL, devid);
            }
        } else if (bus == BUS_GENESIS && base == BASE_GENESIS_TTY) {
            if (sub == SUB_GENESIS_TTY_VIRTUAL) {
                mknod("console", FT_SPECIAL, devid);
            }
        } else if (bus == BUS_ISA && base == BASE_ISA_INTEL) {
            if (sub == SUB_ISA_INTEL_8253) {
                mknod("timer", FT_SPECIAL, devid);
            }
        } else if (bus == BUS_ISA && base == BASE_ISA_IBM) {
            if (sub == SUB_ISA_IBM_VGA) {
                mknod("vga", FT_SPECIAL, devid);
            }
        } else if (bus == BUS_SERIO && base == BASE_i8042_ATKBC) {
            if (sub == SUB_i8042_ATKBC_PS2KEYBOARD) {
                mknod("keyboard", FT_SPECIAL, devid);
            } else if (sub == SUB_i8042_ATKBC_PS2MOUSE) {
                mknod("mouse", FT_SPECIAL, devid);
            }
        } else if (bus == BUS_IDE || bus == BUS_SCSI) {
            if (bus == BUS_IDE  && base == BASE_ATA_DISK ||
                bus == BUS_SCSI && base == BASE_SCSI_DISK) {
                /* hard disk */
                disk_indx = get_free_disk();
                if (disk_indx >= 0) {
                    filename[0] = 's';
                    filename[1] = 'd';
                    filename[2] = disk_indx+'a';
                    filename[3] = 0;
                    mknod(filename, FT_SPECIAL, devid);
                    disks[disk_indx].devid = devid;
                }
            } else if (bus == BUS_IDE && base == BASE_ATAPI_CDROM) {
                /* cdrom */
                strcpy(filename, "sr");
                itoa(cdrom_indx++, &filename[strlen(filename)], 10);
                mknod(filename, FT_SPECIAL, devid);
            }
        } else if (bus == BUS_DISK && base == BASE_DISK_PARTITION) {
            disk_indx = get_disk_by_devid(progif);
            if (disk_indx >= 0) {
                filename[0] = 's';
                filename[1] = 'd';
                filename[2] = disk_indx+'a';
                filename[3] = 0;
                itoa(sub, &filename[strlen(filename)], 10);
                mknod(filename, FT_SPECIAL, devid);
            }
        }
    }

    /* done */
    fclose(dev);
    return 0;
}
