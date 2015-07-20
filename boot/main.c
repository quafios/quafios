/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Boot-Loader.                                | |
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

#include <sys/bootinfo.h>

extern bootinfo_t *bootinfo;

int main() {

    /* enter unreal mode */
    enter_unreal();

    /* print splash */
    printf("\nQuafios boot loader is starting...\n");

    /* show menu */
    show_menu();

    /* clear screen */
    cls();

    /* enable A20 Line */
    enable_a20();

    /* initialize bootinfo */
    bootinfo_init();

    /* decompress the ram disk */
    gunzip("RAMDISK.GZ");

    /* load kernel to memory */
    diskfs_load("/boot/kernel.bin", bootinfo->res[BI_KERNEL].base);

    /* set VGA resolution */
    set_resolution();

    /* enter protected mode & execute kernel.bin */
    go_protected(bootinfo->res[BI_KERNEL].base);

    /* set video mode 0x03: */
    video_mode(0x03);

    /* kernel returned (this should never happen) */
    printf("Quafios kernel main() returned!\n");

    /* done */
    return 0;

}
