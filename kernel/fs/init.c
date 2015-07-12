/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> File Manager.                                    | |
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

#include <arch/type.h>
#include <sys/fs.h>
#include <sys/mm.h>

uint32_t bootdisk = 0xFFFFFFFF; /* TODO: This is a temporary method
                                 *       for boot-disk detection...
                                 */

void fs_init() {

    /* print init message: */
    printk("Quafios filesystem is starting...\n");

    /* initialize caches: */
    icache_init();

    /* mount tmpfs to the root. */
    mount("", "/", "tmpfs", 0, NULL);

    /* create a device file for bootdisk */
    mknod("/bootdisk", FT_SPECIAL, bootdisk);

    /* mount diskfs filesystem */
    mount("/bootdisk", "/", "diskfs", 0, NULL);

}
