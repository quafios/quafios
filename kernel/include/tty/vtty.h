/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> Virtual console header.                          | |
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

/* vtty commands: */
#define TTY_PRESS        0x00 /* called by keyboard driver on key press */
#define TTY_REGVIDEO     0x01 /* Register video driver */
#define TTY_ATTR         0x02 /* Change attribute */
#define TTY_GETCURSOR    0x03 /* get cursor offsets */
#define TTY_SETATTRATOFF 0x04 /* set attribute of a specific offset */
#define TTY_SETCHARATOFF 0x05 /* set character at a specific offset */
#define TTY_SETCURSOR    0x06 /* set cursor offsets */
#define TTY_NOECHO       0x07 /* disable echo */
#define TTY_SETECHO      0x08 /* enable echo */
#define TTY_NOBUF        0x09 /* disable internal buffering */
#define TTY_SETBUF       0x0A /* enable internal buffering */
#define TTY_DISABLE      0x0B /* disable TTY */
#define TTY_ENABLE       0x0C /* enable TTY */
#define TTY_FORK         0x0D /* create a pseudo TTY */

typedef struct {
    char prefix;
    int  devid;
} tty_fork_t;
