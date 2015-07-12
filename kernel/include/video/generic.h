/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> Generic video headers.                           | |
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

#ifndef VGA_GENERIC_H
#define VGA_GENERIC_H

#include <arch/type.h>

/* Colors: */
#define VGA_FG_BLACK            0x00
#define VGA_FG_BLUE             0x01
#define VGA_FG_GREEN            0x02
#define VGA_FG_CYAN             0x03
#define VGA_FG_RED              0x04
#define VGA_FG_MAGENTA          0x05
#define VGA_FG_BROWN            0x06
#define VGA_FG_GRAY_LIGHT       0x07
#define VGA_FG_GRAY_DARK        0x08
#define VGA_FG_BLUE_BRIGHT      0x09
#define VGA_FG_GREEN_BRIGHT     0x0A
#define VGA_FG_CYAN_BRIGHT      0x0B
#define VGA_FG_RED_BRIGHT       0x0C
#define VGA_FG_MAGENTA_BRIGHT   0x0D
#define VGA_FG_YELLOW           0x0E
#define VGA_FG_WHITE            0x0F

#define VGA_BG_BLACK            0x00
#define VGA_BG_BLUE             0x10
#define VGA_BG_GREEN            0x20
#define VGA_BG_CYAN             0x30
#define VGA_BG_RED              0x40
#define VGA_BG_MAGENTA          0x50
#define VGA_BG_BROWN            0x60
#define VGA_BG_GRAY_LIGHT       0x70
#define VGA_BG_GRAY_DARK        0x80
#define VGA_BG_BLUE_BRIGHT      0x90
#define VGA_BG_GREEN_BRIGHT     0xA0
#define VGA_BG_CYAN_BRIGHT      0xB0
#define VGA_BG_RED_BRIGHT       0xC0
#define VGA_BG_MAGENTA_BRIGHT   0xD0
#define VGA_BG_YELLOW           0xE0
#define VGA_BG_WHITE            0xF0

/* vga commands: */
#define VGA_GET_WIDTH           0x00
#define VGA_GET_HEIGHT          0x01
#define VGA_GET_DEPTH           0x02
#define VGA_GET_SCANLINE        0x03
#define VGA_PLOT                0x04

typedef struct {

    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    uint8_t *buf;

} vga_plot_t;

#endif
