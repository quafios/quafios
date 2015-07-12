/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios GUI Library.                                | |
 *        | |  -> Window header.                                   | |
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

#ifndef _GUI_WINDOW_H
#define _GUI_WINDOW_H

typedef struct window {

    /* next window */
    struct window *next;

    /* window ID */
    int wid;

    /* title */
    char title[100];

    /* icon */
    char iconfile[100];

    /* background color */
    unsigned int bg;

    /* is this the main window? */
    int mainwin;

    /* pixel buffer */
    pixbuf_t *pixbuf;

    /* components */
    struct {
        int   x;
        int   y;
        int width;
        int height;
        void *element;
    } components[100]; /* maximum count */

    /* events */
    void (*press)(char btn);
    void (*click)(unsigned int x, unsigned int y);
    int (*close)(void);

} window_t;

void get_win_shfname(char *buf, int pid, int wid);
window_t *get_window();
window_t *window_alloc(char *title,
                       unsigned int width,
                       unsigned int height,
                       unsigned int x,
                       unsigned int y,
                       unsigned int bg,
                       char *iconfile);
void window_flush(window_t *window,
                  unsigned int x,
                  unsigned int y,
                  unsigned int width,
                  unsigned int height);
void window_insert(window_t *window, void *element, int x, int y);
void window_press(window_t *window, unsigned char btn);
#endif
