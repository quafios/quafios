/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Window Manager.                             | |
 *        | |  -> VGA interface.                                   | |
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

#include <gui.h>
#include <api/fs.h>
#include <video/generic.h>
#include <tty/vtty.h>

#include "winman.h"

/* VGA file descriptor */
int vga_fd;

/* VGA mode */
unsigned int vga_mode;

/* VGA parameters */
unsigned int vga_width;
unsigned int vga_height;

unsigned int vga_get_width() {

    /* return VGA width */
    return vga_width;

}

unsigned int vga_get_height() {

    /* return VGA height */
    return vga_height;

}

void vga_plot(pixbuf_t *pixbuf, unsigned int x, unsigned int y) {

    /* create the plot structure and send it to VGA */
    vga_plot_t plot;
    plot.x = x;
    plot.y = y;
    plot.width = pixbuf->width;
    plot.height = pixbuf->height;
    plot.buf = (unsigned char *) pixbuf->buf;
    ioctl(vga_fd, VGA_PLOT, &plot);

}

void vga_init() {

    /* open the vga driver */
    vga_fd = open("/dev/vga", 0);

    /* get VGA mode */
    ioctl(vga_fd, VGA_GET_MODE,  &vga_mode);

    /* if text mode, exit winman */
    if (!vga_mode)
        exit(0);

    /* get vga parameters */
    ioctl(vga_fd, VGA_GET_WIDTH,  &vga_width);
    ioctl(vga_fd, VGA_GET_HEIGHT, &vga_height);

    /* enter graphics mode */
    ioctl(1, TTY_DISABLE, NULL);

}
