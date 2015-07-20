/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Boot information structures.                     | |
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

#ifndef BOOTINFO_H
#define BOOTINFO_H

#include <arch/type.h>

typedef struct {
    uint64_t base;
    uint64_t end;
} mentry_t;

typedef struct {

    /* live disk parameters: */
    uint32_t live; /* 0: not live, 1: live ramdisk. */

    /* VGA info */
    uint32_t vga_width;
    uint32_t vga_height;
    uint32_t vga_depth;
    uint32_t vga_phys;     /* linear frame buffer */
    uint32_t vga_scanline; /* bytes per scan line */
    uint32_t vga_mode;     /* 0: text mode, 1: graphics mode */

    /* boot disk info: */
    /* TODO.           */

    /* reserved RAM: */
#define BI_BOOTLOADER   0
#define BI_BOOTINFO     1
#define BI_KERNEL       2
#define BI_RAMDISK      3
#define BI_ARCH0        4
#define BI_ARCH1        5
#define BI_ARCH2        6

#define BI_RESCOUNT     7
    mentry_t res[BI_RESCOUNT];

    /* total valid RAM: */
    uint32_t mem_ents;     /* count of valid RAM entries. */
    mentry_t mem_ent[1];   /* array of valid RAM entries. */

} bootinfo_t;

#endif
