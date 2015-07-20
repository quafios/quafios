/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> i8042 chip header.                               | |
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

/* Firmware codes:  */
/* ---------------- */
#define i8042_nofirmware        0x0000 /* i8042 when no specific firmware
                                          is loaded on it */
#define i8042_ATKBC             0x0001 /* i8042 acting as a PS/2 Keyboard
                                          controller on PC systems. */

/* Firmware commands:  */
/* ------------------- */
/* AT Keyboard Controller on i8042: */
#define i8042_ATKBC_INIT        0x0000
#define i8042_ATKBC_SENDREC     0x0001
typedef struct {
    uint32_t channel;
    uint8_t  send;    /* count of bytes to send to the interface. */
    uint8_t* sbuff;   /* send buffer.                             */
    uint32_t receive; /* count of bytes to receive.               */
    uint8_t* rbuff;   /* buffer to receive data in.               */
} atkbc_sendrec_t;

/* Initialization Data: */
typedef struct {
    uint32_t firmware_code; /* the firmware loaded on the chip. */
} i8042_init_t;
