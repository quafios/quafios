/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Mouse device driver header (generic).            | |
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

#ifndef MOUSE_GENERIC_H
#define MOUSE_GENERIC_H

#include <arch/type.h>

#define MOUSE_GETX      0
#define MOUSE_GETY      1
#define MOUSE_REG       2

typedef struct {

    uint8_t prefix;
    int32_t mouse_x;
    int32_t mouse_y;
    int32_t left_btn;
    int32_t mid_btn;
    int32_t right_btn;
    uint64_t time;

} __attribute__((packed)) mouse_packet_t;

#endif

