/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios GUI Library.                                | |
 *        | |  -> Window procedures.                               | |
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
#include <stdlib.h>
#include <api/proc.h>
#include <api/mman.h>

/* linked list of windows */
window_t *firstwin = NULL;

int last_wid = 0;

void get_win_shfname(char *buf, int pid, int wid) {

    /* get window shared file name */
    strcpy(buf, "/run/win_");
    itoa(pid, buf + strlen(buf), 10);
    strcpy(buf + strlen(buf), "_");
    itoa(wid, buf + strlen(buf), 10);

}

window_t *get_window(int wid) {

    window_t *ptr = firstwin;
    while (ptr && ptr->wid != wid)
        ptr = ptr->next;
    return ptr;

}

window_t *window_alloc(char *title,
                       unsigned int width,
                       unsigned int height,
                       unsigned int x,
                       unsigned int y,
                       unsigned int bg,
                       char *iconfile) {

    char fname[255];
    int wid = ++last_wid;
    int fd;
    unsigned int *buf;
    window_t *window;
    wm_reg_t req;
    int dx, dy, i;

    /* get window shared file name */
    get_win_shfname(fname, getpid(), wid);

    /* create the shared file */
    mknod(fname, 0x0777, 0);

    /* open the shared file */
    fd = open(fname, 0);

    /* map the pixel buffer */
    buf = mmap(NULL, width*height*4, MMAP_TYPE_FILE,
               MMAP_FLAGS_SHARED, fd, 0);

    /* close the shared file */
    close(fd);

    /* allocate the window structure */
    window = malloc(sizeof(window_t));

    /* initialize the window structure */
    strcpy(window->title, title);
    strcpy(window->iconfile, iconfile);
    window->bg = bg;
    window->mainwin = 0;
    window->wid = wid;
    window->press = NULL;
    window->click = NULL;
    window->close = NULL;

    /* allocate the pixel buffer */
    window->pixbuf = malloc(sizeof(pixbuf_t));

    /* initialize the pixel buffer */
    window->pixbuf->width = width;
    window->pixbuf->height = height;
    window->pixbuf->buf = buf;

    /* draw background */
    for (dy = 0; dy < height; dy++)
        for (dx = 0; dx < width; dx++)
            pixbuf_set_pixel(window->pixbuf, dx, dy, window->bg);

    /* no components */
    for (i = 0; i < 100; i++)
        window->components[i].element = NULL;

    /* tell winman about the new window */
    req.prefix = 0;
    req.type = WMREQ_REGWIN;
    req.id = wid;
    req.x = x;
    req.y = y;
    req.width = width;
    req.height = height;
    req.visible = 0;
    strcpy(req.title, title);
    strcpy(req.iconfile, iconfile);
    wm_req(&req, sizeof(wm_reg_t));

    /* add to linked list */
    window->next = firstwin;
    firstwin = window;

    /* done */
    return window;

}

void window_flush(window_t *window,
                  unsigned int x,
                  unsigned int y,
                  unsigned int width,
                  unsigned int height) {

    /* redraw that part of the window */
    wm_redraw_t req;
    req.prefix = 0;
    req.type = WMREQ_REDRAW;
    req.id = window->wid;
    req.x = x;
    req.y = y;
    req.width = width;
    req.height = height;
    wm_req(&req, sizeof(wm_redraw_t));

}

void window_insert(window_t *window, void *element, int x, int y) {

    int i = 0;
    if (((int *) element)[0] == 0) {
        /* iconview */
        iconview_t *t = element;
        while (window->components[i].element)
            i++;
        window->components[i].x = x;
        window->components[i].y = y;
        window->components[i].width   = t->width;
        window->components[i].height  = t->height;
        window->components[i].element = element;
        t->parent = window;
        t->x = x;
        t->y = y;
        /* allocate an active pxibuf for iconview */
        t->active = pixbuf_alloc(t->width, t->height);
        /* redraw iconview */
        iconview_redraw(t);
    }

}

void window_press(window_t *window, unsigned char btn) {

    /* execute event */
    if (window->press)
        window->press(btn);

}

void window_click(window_t *window, unsigned int x, unsigned int y) {

    int i;

    if (window->click)
        window->click(x, y);

    /* look for a component to click on */
    for (i = 0; window->components[i].element; i++) {
        if (x >= window->components[i].x &&
            y >= window->components[i].y &&
            x < window->components[i].x + window->components[i].width &&
            y < window->components[i].y + window->components[i].height) {

            /* the component found */
            if (((int *) window->components[i].element)[0] == 0) {
                /* iconview */
                iconview_click(window->components[i].element, x, y);
            }
        }
    }

}

void window_double(window_t *window, unsigned int x, unsigned int y) {

    int i;
    /* look for a component to click on */
    for (i = 0; window->components[i].element; i++) {
        if (x >= window->components[i].x &&
            y >= window->components[i].y &&
            x < window->components[i].x + window->components[i].width &&
            y < window->components[i].y + window->components[i].height) {

            /* the component found */
            if (((int *) window->components[i].element)[0] == 0) {
                /* iconview */
                iconview_double(window->components[i].element, x, y);
            }
        }
    }

}

void window_close(window_t *window) {

    wm_closewin_t req;
    window_t *ptr = firstwin;
    window_t *prev = NULL;
    int i;

    /* execute the event handler */
    if (window->close)
        if (window->close())
            return;

    /* send request to winman */
    req.prefix = 0;
    req.type   = WMREQ_CLOSEWIN;
    req.id     = window->wid;
    wm_req(&req, sizeof(wm_reg_t));

    /* remove from linked list */
    while (ptr && ptr != window)
        ptr = (prev = ptr)->next;
    if (!prev)
        firstwin = window->next;
    else
        prev->next = window->next;

    /* destroy all components */
    for (i = 0; window->components[i].element; i++)
        /* TODO */;

    /* terminate GUI? */
    if (window->mainwin)
        while(firstwin)
            window_close(firstwin);

    /* deallocate window */
    free(window->pixbuf);
    free(window);

}
