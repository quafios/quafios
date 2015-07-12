/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> Pseudo TTY header.                               | |
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

/* pstty commands (sent to the program in pstty_packet_t) */
#define PSTTY_PUTC              0
#define PSTTY_CHANGE_ATTR       1
#define PSTTY_SET_ATTR_AT_OFF   2
#define PSTTY_SET_CHAR_AT_OFF   3
#define PSTTY_SET_CURSOR        4
#define PSTTY_GET_CURSOR        5

typedef struct {
    int pid;
    char prefix;
} __attribute__((packed)) pstty_init_t;

typedef struct {
    char prefix;
    char cmd;
    char chr;
    char attr;
    char x;
    char y;
} __attribute__((packed)) pstty_packet_t;
