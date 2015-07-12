/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Boot-Loader.                                | |
 *        | |  -> main() procedure.                                | |
 *        | +------------------------------------------------------+ |
 *        +----------------------------------------------------------+
 *
 * This file is part of Quafios 1.0.2 source code.
 * Copyright (C) 2014  Mostafa Abd El-Aziz Mohamed.
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

#include <sys/bootinfo.h>

extern bootinfo_t *bootinfo;

int main() {

    /* Enter Unreal Mode: */
    enter_unreal();

    /* Print splash: */
    printf("\nQuafios boot loader is starting...\n");

    /* Enable A20 Line: */
    enable_a20();

    /* Initialize bootinfo: */
    bootinfo_init();

    /* Decompress the ram disk: */
    gunzip("RAMDISK.GZ");

    /* Load kernel to memory: */
    diskfs_load("/boot/kernel.bin", bootinfo->res[BI_KERNEL].base);

    /* set VGA resolution */
    set_resolution();

    /* Enter protected mode & execute kernel.bin: */
    go_protected(bootinfo->res[BI_KERNEL].base);

    /* Set video mode 0x03: */
    video_mode(0x03);

    /* Kernel returned (this should never happen): */
    printf("Quafios kernel main() returned!\n");

    /* Done: */
    return 0;

}
