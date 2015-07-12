/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Window Manager.                             | |
 *        | |  -> header file.                                     | |
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

#ifndef WINMAN_H
#define WINMAN_H

#include <gui.h>
#include <mouse/generic.h>
#include <keyboard/generic.h>

typedef struct win_info {
    struct win_info *next;
    int pid;
    int wid;
    unsigned int x;
    unsigned int y;
    char title[100];
    char iconfile[100];
    int visible;
    pixbuf_t *pixbuf;
    pixbuf_t *ico;
} win_info_t;

unsigned int vga_get_width();
unsigned int vga_get_height();
void vga_plot(pixbuf_t *pixbuf, unsigned int x, unsigned int y);

void keyboard_event(keyboard_packet_t *packet);

void mouse_event(mouse_packet_t *packet);

pixbuf_t *get_osm();

win_info_t *get_top_window();

void draw_window(win_info_t *win_info, int plot_on_vga, int redraw_mouse);
win_info_t *change_focus(int x, int y);
void window_request(int pid, unsigned char *req);

#endif
