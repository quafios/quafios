/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios GUI Library.                                | |
 *        | |  -> Window manager interface header.                 | |
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

#ifndef WINMANIF_H
#define WINMANIF_H

#define WMREQ_REGWIN    1 /* register a window at winman */
#define WMREQ_REDRAW    2 /* redraw part of the winow    */
#define WMREQ_CLOSEWIN  3 /* unregister window at winman */
#define WMREQ_SET_WP    4 /* set wallpaper               */

#define WMEVENT_DRAW    1 /* draw window event           */
#define WMEVENT_PRESS   2 /* keyboard button pressed     */
#define WMEVENT_CLICK   3 /* mouse button clicked        */
#define WMEVENT_DOUBLE  4 /* mouse button double clicked */
#define WMEVENT_CLOSE   5 /* close button clicked        */

#define PREFIX_APPLICATION      0
#define PREFIX_MOUSE            1
#define PREFIX_KEYBOARD         2
#define PREFIX_WINMAN           3

typedef struct wm_reg {
    unsigned char prefix; /* should be zero              */
    unsigned char type;   /* request type                */
    unsigned int id;      /* local handler of the window */
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
    unsigned int visible;
    char title[100];
    char iconfile[100];
} __attribute__((packed)) wm_reg_t;

typedef struct wm_redraw {
    unsigned char prefix; /* should be zero              */
    unsigned char type;   /* request type                */
    unsigned int id;      /* local handler of the window */
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
} __attribute__((packed)) wm_redraw_t;

typedef struct wm_closewin {
    unsigned char prefix; /* should be zero              */
    unsigned char type;   /* request type                */
    unsigned int id;      /* local handler of the window */
} __attribute__((packed)) wm_closewin_t;

typedef struct wm_set_wallpaper {
    unsigned char prefix; /* should be zero              */
    unsigned char type;   /* request type                */
    unsigned int id;      /* local handler of the window */
    unsigned int index;   /* which wallpaper             */
} __attribute__((packed)) wm_set_wallpaper_t;

typedef struct wm_event {
    unsigned char prefix; /* prefix to detect the packet source */
    unsigned int type;    /* event type                  */
    unsigned int id;      /* local handler of the window */
    unsigned int data[8];
} __attribute__((packed)) wm_event_t;

int wm_req(void *req, int size);
void set_receiver(void (*rec)(void *packet));
void gui_loop();

#endif
