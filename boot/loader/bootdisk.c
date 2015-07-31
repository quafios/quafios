/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Boot-Loader.                                | |
 *        | |  -> boot disk access layer.                          | |
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
#include <sys/bootinfo.h>

typedef struct dap {
    uint8_t  size;
    uint8_t  res;
    uint16_t count;
    uint16_t offset;
    uint16_t segment;
    uint32_t lba_low;
    uint32_t lba_high;
} __attribute__((packed)) dap_t;

uint8_t  *drivenum    = (uint8_t  *) 0x7DF0;
uint32_t *ramdiskoff  = (uint32_t *) 0x7DF1;
uint32_t *ramdisksize = (uint32_t *) 0x7DF5;
uint32_t *partstart   = (uint32_t *) 0x7DF9;
uint8_t  *fstype      = (uint8_t  *) 0x7DFD;

extern bootinfo_t *bootinfo;

void read_sectors(uint32_t lba, int16_t count, uint32_t addr) {
    dap_t dap = {16, 0, 0, 0, 0, 0, 0};
    int32_t i;
    uint32_t src;
    if (*drivenum == 0xFF) {
        /* ramdisk */
        src = (*partstart + lba)*512;
        for(i = 0; i < count*512; i++)
            ((uint8_t *) addr)[i] = ((uint8_t *) (*ramdiskoff+src))[i];
    } else {
        dap.count = count;
        dap.offset = addr;
        dap.lba_low = *partstart + lba;
        __asm__("int $0x13"::"a"(0x4200),"d"(*drivenum),"S"(&dap));
    }
}

void bootdisk_init() {

    /* set bootdisk data in bootinfo structure */
    if (*drivenum == 0xFF) {
        /* live (indicates whether there is a ramdisk) */
        bootinfo->live = 1;
        /* store ramdisk info in bootinfo structure */
        bootinfo->res[BI_RAMDISK].base = *ramdiskoff;
        bootinfo->res[BI_RAMDISK].end  = *ramdiskoff+*ramdisksize;
    } else {
        /* no ramdisk */
        bootinfo->live = 0;
    }

}
