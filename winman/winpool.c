/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Window Manager.                             | |
 *        | |  -> Window pool.                                     | |
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

#include <stdlib.h>
#include <gui.h>
#include <api/proc.h>
#include <api/mman.h>

#include "winman.h"

win_info_t *first = NULL;
int wincount = 0;
int ack = 0;

pixbuf_t *min;
pixbuf_t *max;
pixbuf_t *clos;

int rand = 0;

win_info_t *get_top_window() {
    return first;
}

win_info_t *get_win_info(int pid, int wid) {

    /* get win_info_t structure */
    win_info_t *ptr = first;
    while (ptr != NULL && (ptr->pid != pid || ptr->wid != wid))
        ptr = ptr->next;
    return ptr;

}

void redraw_window(int pid, wm_redraw_t *req) {

    /* redraw part of the window */
    win_info_t *win;
    pixbuf_t *redraw;
    int alloc = 0;

    /* get the window structure */
    win = get_win_info(pid, req->id);

    /* allocate redraw */
    if (req->x == 0 && req->y == 0 && req->width == win->pixbuf->width &&
        req->height == win->pixbuf->height) {

        /* redraw the whole window */
        redraw = win->pixbuf;

    } else {

        /* allocate redraw */
        redraw = pixbuf_alloc(req->width, req->height);

        /* crop the part that is to be updated */
        pixbuf_crop(win->pixbuf, redraw, req->x, req->y);

        /* set the flag */
        alloc = 1;

    }

    /* update on osm */
    pixbuf_paint(redraw, get_osm(), win->x + req->x, win->y + req->y);

    /* tell the vga to plot it */
    vga_plot(redraw, win->x + req->x, win->y + req->y);

    /* update mouse */
    draw_cursor();

    /* deallocate redraw */
    if (alloc) {
        free(redraw->buf);
        free(redraw);
    }

}

void draw_window(win_info_t *win_info, int plot_on_vga, int redraw_mouse) {

    pixbuf_t *osm = get_osm();
    pixbuf_t *full_window;
    int x, y, w_width, w_height, width, height;

    /* calculate full_window parameters */
    x = win_info->x - 4;
    y = win_info->y - 22;
    w_width = win_info->pixbuf->width;
    w_height = win_info->pixbuf->height;
    width = w_width + 8;
    height = w_height + 22 + 4;

    /* allocate pixbuf for full_window */
    full_window = pixbuf_alloc(width, height);

    /* draw title bar */
    draw_solid(full_window, 0, 0, width,  11, 0xFF862030);
    draw_solid(full_window, 0, 11, width, 11, 0xFF611E2D);

    /* draw borders */
    draw_solid(full_window, 0, 22, 4, w_height, 0xFF611E2D);
    draw_solid(full_window, 4+w_width, 22, 4, w_height, 0xFF611E2D);
    draw_solid(full_window, 0, 22+w_height, w_width+4+4, 4, 0xFF611E2D);

    /* draw title */
    draw_text(full_window, (w_width-strlen(win_info->title)*9-30)/2, 4,
              win_info->title, 0xFFFFFFFF);

    /* draw buttons and icons */
    pixbuf_paint(min,  full_window, width-74, 3);
    pixbuf_paint(max,  full_window, width-50, 3);
    pixbuf_paint(clos, full_window, width-26, 3);
    pixbuf_paint(win_info->ico, full_window, 5, 4);

    /* draw the screen content */
    pixbuf_paint(win_info->pixbuf, full_window, 4, 22);

    /* paint full_window on osm */
    pixbuf_paint(full_window, get_osm(), x, y);

    /* paint on screen */
    if (plot_on_vga)
        vga_plot(full_window, x, y);

    /* draw the mouse cursor */
    if (redraw_mouse)
        draw_cursor();

    /* deallocate full_window */
    free(full_window->buf);
    free(full_window);

}

void add_window(int pid, wm_reg_t *req) {

    char fname[255];
    int fd;
    unsigned int *buf;
    win_info_t *win_info;

    /* allocate win_info structure */
    win_info = malloc(sizeof(win_info_t));

    /* get window shared file name */
    get_win_shfname(fname, pid, req->id);

    /* open the shared file */
    fd = open(fname, 0);

    /* map the pixel buffer */
    buf = mmap(NULL, req->width*req->height*4, MMAP_TYPE_FILE,
               MMAP_FLAGS_SHARED, fd, 0);

    /* close the shared file */
    close(fd);

    /* initialize win_info structure */
    win_info->next    = first;
    win_info->pid     = pid;
    win_info->wid     = req->id;
    win_info->visible = req->visible;
    strcpy(win_info->title, req->title);
    strcpy(win_info->iconfile, req->iconfile);
    win_info->ico = parse_png(win_info->iconfile);

    /* allocate pixel buffer structure */
    win_info->pixbuf = malloc(sizeof(pixbuf_t));

    /* initialize pixbuf */
    win_info->pixbuf->width  = req->width;
    win_info->pixbuf->height = req->height;
    win_info->pixbuf->buf    = buf;

    /* x and y */
    if (req->x == -1) {
        /* generate random offsets */
        rand += 100;
        if (rand >= vga_get_width() || rand >= vga_get_height())
            rand = 100;
        win_info->x = rand;
        win_info->y = rand;
    } else if (req->x == -2) {
        win_info->x = (get_osm()->width-req->width)/2;
        win_info->y = (get_osm()->height-req->height)/2;
    } else {
        win_info->x = req->x;
        win_info->y = req->y;
    }

    /* add to the linked list */
    first = win_info;
    wincount++;

    /* update screen if window is visible */
    /* if (req->visible) */
        draw_window(win_info, 1, 1);

}

win_info_t *change_focus(int x, int y) {

    /* search for the first window that has x & y inside */
    win_info_t *ptr = first;
    win_info_t *prev = NULL;
    while (ptr != NULL) {
        int actual_x = ptr->x-4;
        int actual_y = ptr->y-22;
        int actual_width = ptr->pixbuf->width + 8;
        int actual_height = ptr->pixbuf->height + 22+4;

        if (x >= actual_x && x < (actual_x + actual_width) &&
            y >= actual_y && y < (actual_y + actual_height)) {
            /* found */
            if (prev == NULL) {
                first = ptr->next;
            } else {
                prev->next = ptr->next;
            }
            ptr->next = first;
            first = ptr;
            draw_window(ptr, 1, 1);
            return ptr;
        }

        prev = ptr;
        ptr = ptr->next;
    }

    return first;

}

void draw_windows() {


    win_info_t **windows = malloc(sizeof(win_info_t *) * wincount);
    win_info_t *ptr = first;
    int i = 0;

    /* get all windows */
    while (ptr) {
        windows[i++] = ptr;
        /* next window */
        ptr = ptr->next;
    }

    while (i > 0) {
        /* plot the window onto OSM */
        draw_window(windows[--i], 0, 0);
    }

}

void update_screen() {

    /* draw wallpaper on the OSM */
    draw_wallpaper();

    /* draw all windows */
    draw_windows();

    /* draw osm to vga */
    vga_plot(get_osm(), 0, 0);

    /* draw the mouse cursor */
    draw_cursor();

}

void close_window(int pid, wm_closewin_t *req) {

    /* close the window */
    win_info_t *win = get_win_info(pid, req->id);
    win_info_t *ptr = first;
    win_info_t *prev = NULL;

    if (!win)
        return;

    /* remove from linked list */
    while (ptr && ptr != win)
        ptr = (prev = ptr)->next;
    if (!prev)
        first = win->next;
    else
        prev->next = win->next;

    /* deallocate */
    free(win->pixbuf);
    free(win->ico->buf);
    free(win->ico);
    free(win);

    /* redraw screen */
    update_screen();

}

void window_request(int pid, unsigned char *req) {

    /* window request */
    if (req[1] == WMREQ_REGWIN)
        add_window(pid, (wm_reg_t *) req);
    else if (req[1] == WMREQ_REDRAW)
        redraw_window(pid, (wm_redraw_t *) req);
    else if (req[1] == WMREQ_CLOSEWIN)
        close_window(pid, (wm_closewin_t *) req);
    else if (req[1] == WMREQ_SET_WP) {
        set_wallpaper(((wm_set_wallpaper_t *) req)->index);
        update_screen();
    }

}

void winpool_init() {

    min = parse_png("/usr/share/icons/min.png");
    max = parse_png("/usr/share/icons/max.png");
    clos = parse_png("/usr/share/icons/close.png");

}
