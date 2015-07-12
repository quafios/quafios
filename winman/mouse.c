/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Window Manager.                             | |
 *        | |  -> Mouse cursor drawing and movement.               | |
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

#include <gui.h>
#include <api/fs.h>
#include <api/proc.h>
#include <mouse/generic.h>
#include <video/generic.h>

#include "winman.h"

pixbuf_t *cursor;
pixbuf_t *tmp; /* a pixbuf for temporary operations */
int mouse_fd;

int old_x = 0;
int old_y = 0;

int old_leftbtn = 0;

uint64_t last_click = -1;

/* drag and drop */
int drag_x = -1;
int drag_y = -1;
int win_x = -1;
int win_y = -1;
int last_x = -1;
int last_y = -1;

void draw_cursor() {

    int i, j;
    int mouse_x = 0, mouse_y = 0;
    extern int vga_fd;
    vga_plot_t plot;

    /* get cursor position */
    ioctl(mouse_fd, MOUSE_GETX, &mouse_x);
    ioctl(mouse_fd, MOUSE_GETY, &mouse_y);

    /* recover the old region */
    pixbuf_crop(get_osm(), tmp, old_x, old_y);

    /* draw the old region to vga */
    vga_plot(tmp, old_x, old_y);

    /* update mouse pointer */
    old_x = mouse_x;
    old_y = mouse_y;

    /* get osm region */
    pixbuf_crop(get_osm(), tmp, mouse_x, mouse_y);

    /* paint the cursor over this region */
    pixbuf_paint(cursor, tmp, 0, 0);

    /* draw the region with cursor over it to vga */
    vga_plot(tmp, mouse_x, mouse_y);

}

int abs(int x) {
    return x > 0 ? x : -x;
}

void show_box(int x1, int y1, int width, int height) {

    int i;

    pixbuf_t *line1 = pixbuf_alloc(width, 1);
    pixbuf_t *line2 = pixbuf_alloc(width, 1);
    pixbuf_t *line3 = pixbuf_alloc(1, height);
    pixbuf_t *line4 = pixbuf_alloc(1, height);

    pixbuf_crop(get_osm(), line1, x1, y1);
    for (i = 0; i < line1->width * line1->height; i++)
        line1->buf[i] = 0x00FFFFFF ^ line1->buf[i];
    vga_plot(line1, x1, y1);

    pixbuf_crop(get_osm(), line2, x1, y1+height-1);
    for (i = 0; i < line2->width * line2->height; i++)
        line2->buf[i] = 0x00FFFFFF ^ line2->buf[i];
    vga_plot(line2, x1, y1+height-1);

    pixbuf_crop(get_osm(), line3, x1, y1);
    for (i = 0; i < line3->width * line3->height; i++)
        line3->buf[i] = 0x00FFFFFF ^ line3->buf[i];
    vga_plot(line3, x1, y1);

    pixbuf_crop(get_osm(), line4, x1+width-1, y1);
    for (i = 0; i < line4->width * line4->height; i++)
        line4->buf[i] = 0x00FFFFFF ^ line4->buf[i];
    vga_plot(line4, x1+width-1, y1);

}

void hide_box(int x1, int y1, int width, int height) {

    int i;

    pixbuf_t *line1 = pixbuf_alloc(width, 1);
    pixbuf_t *line2 = pixbuf_alloc(width, 1);
    pixbuf_t *line3 = pixbuf_alloc(1, height);
    pixbuf_t *line4 = pixbuf_alloc(1, height);

    pixbuf_crop(get_osm(), line1, x1, y1);
    vga_plot(line1, x1, y1);

    pixbuf_crop(get_osm(), line2, x1, y1+height-1);
    vga_plot(line2, x1, y1+height-1);

    pixbuf_crop(get_osm(), line3, x1, y1);
    vga_plot(line3, x1, y1);

    pixbuf_crop(get_osm(), line4, x1+width-1, y1);
    vga_plot(line4, x1+width-1, y1);

}

void mouse_event(mouse_packet_t *packet) {

    if (packet->mouse_x != old_x || packet->mouse_y != old_y) {

        /* mouse moved */
        if (old_leftbtn || packet->left_btn) {
            /* the user is dragging something! */
            int dx, dy;
            win_info_t *win = get_top_window();

            if (win && (drag_x != -1 || (
                (int) old_x >= (int) win->x-4  &&
                (int) old_y >= (int) win->y-22 &&
                (int) old_x < (int) win->x+win->pixbuf->width+8 &&
                (int) old_y < (int) win->y))) {

                /* the dragging is on the title bar */
                if (drag_x == -1) {
                    drag_x = old_x;
                    drag_y = old_y;
                    win_x = win->x;
                    win_y = win->y;
                }

                if (last_x != -1) {
                    dx = last_x - drag_x;
                    dy = last_y - drag_y;

                    hide_box(win_x-4 + dx, win_y-22 + dy,
                            win->pixbuf->width+8, win->pixbuf->height+26);

                }

                dx = (last_x = packet->mouse_x) - drag_x;
                dy = (last_y = packet->mouse_y) - drag_y;

                show_box(win_x-4 + dx, win_y-22 + dy,
                        win->pixbuf->width+8, win->pixbuf->height+26);
            }
        }

        draw_cursor();

    }

    if (!old_leftbtn && packet->left_btn) {

        /* button pressed */
        old_leftbtn = 1;

        if (drag_x == -1) {

            win_info_t *top;
            msg_t msg;
            wm_event_t event = {0};

            /* change focus */
            top = change_focus(old_x, old_y);
            if (top) {
                /* send event packet */
                int rx = old_x - (top->x-4);
                int ry = old_y - (top->y-22);
                int close_x = (top->pixbuf->width+8)-26;
                int close_y = 3;
                int close_width = 20;
                int close_height = 16;
                event.prefix  = PREFIX_WINMAN;
                if (rx >= close_x && ry >= close_y &&
                    rx < close_x + close_width &&
                    ry < close_y + close_height) {
                    /* close button clicked */
                    event.type    = WMEVENT_CLOSE;
                } else if (last_click == -1 ||
                           packet->time - last_click > 500) {
                    /* simple click */
                    event.type    = WMEVENT_CLICK;
                } else {
                    /* double click */
                    event.type    = WMEVENT_DOUBLE;
                }
                last_click = packet->time;
                event.id      = top->wid;
                event.data[0] = old_x - top->x;
                event.data[1] = old_y - top->y;
                msg.buf  = &event;
                msg.size = sizeof(wm_event_t);
                send(top->pid, &msg);
            }

        }
    }

    if (old_leftbtn && !packet->left_btn) {
        /* button released */
        old_leftbtn = 0;
        if (drag_x != -1) {
            /* drop */
            int dx, dy;
            win_info_t *win = get_top_window();
            if (win) {
                dx = packet->mouse_x - drag_x;
                dy = packet->mouse_y - drag_y;
                win->x = win_x + dx;
                win->y = win_y + dy;
                update_screen();
            }
        }
        last_x = -1;
        drag_x = -1;

    }

}

void mouse_init() {

    /* open the mouse driver */
    mouse_fd = open("/dev/mouse", 0);

    /* read cursor PNG */
    cursor = parse_png("/usr/share/icons/mouse.png");

    /* create the temporary pixbuf */
    tmp = pixbuf_alloc(cursor->width, cursor->height);

    /* register winman at mouse driver */
    ioctl(mouse_fd, MOUSE_REG, (void *) PREFIX_MOUSE);

}
