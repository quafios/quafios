/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Boot-Loader.                                | |
 *        | |  -> bootinfo structure.                              | |
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

bootinfo_t *bootinfo = (bootinfo_t *) 0x10000;

void bootinfo_init() {

    uint32_t cont = 0;

    /* read memory layout: */
    bootinfo->mem_ents = 0;

    do {

        struct {
            uint64_t base;
            uint64_t len;
            uint16_t type;
            uint16_t acpi;
        } __attribute__((packed)) smap_entry = {0};
        uint32_t sign, bytes;

        __asm__("int $0x15":"=a"(sign),"=b"(cont),"=c"(bytes)
                           :"D"(&smap_entry),"d"(0x534D4150 /*SMAP*/),
                            "c"(0x14),"b"(cont),"a"(0x0000E820));
        if (sign != 0x534D4150) {
            printf("Can't read BIOS memory map!\n");
            while(1);
        }

        if (smap_entry.type == 1) {
            int32_t i = bootinfo->mem_ents++;
            bootinfo->mem_ent[i].base = smap_entry.base;
            bootinfo->mem_ent[i].end  = smap_entry.base+smap_entry.len;
        }

    } while (cont);

    /* determine RAM regions that should be reserved: */
    bootinfo->res[BI_BOOTLOADER].base = 0x08000;
    bootinfo->res[BI_BOOTLOADER].end  = 0x10000;

    bootinfo->res[BI_BOOTINFO].base   = 0x10000;
    bootinfo->res[BI_BOOTINFO].end    = 0x10000 + sizeof(bootinfo_t) +
                                        sizeof(mentry_t)*bootinfo->mem_ents;

    bootinfo->res[BI_KERNEL].base     = 0x0100000;
    bootinfo->res[BI_KERNEL].end      = 0x0100000; /* temp value */

    bootinfo->res[BI_RAMDISK].base    = 0x0000000;
    bootinfo->res[BI_RAMDISK].end     = 0x0000000; /* temp value */

    bootinfo->res[BI_ARCH0].base      = 0x0000000;
    bootinfo->res[BI_ARCH0].end       = 0x0100000; /* lower 1MB. */

    bootinfo->res[BI_ARCH1].base      = 0x0000000;
    bootinfo->res[BI_ARCH1].end       = 0x0000000;

    bootinfo->res[BI_ARCH2].base      = 0x0000000;
    bootinfo->res[BI_ARCH2].end       = 0x0000000;

}
