/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios GUI Library.                                | |
 *        | |  -> Iconview header.                                 | |
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

#ifndef _GUI_ICONVIEW_H
#define _GUI_ICONVIEW_H

typedef struct {

    /* iconview */
    unsigned int type;

    /* count of icons */
    unsigned int count;

    /* next icon to be inserted */
    unsigned int cur;

    /* icon size */
    unsigned int icon_width;
    unsigned int icon_height;

    /* actual icon size */
    unsigned int each_width;
    unsigned int each_height;

    /* total size */
    unsigned int icons_per_row;
    unsigned int rows;

    /* width and height inside the window */
    unsigned int width;
    unsigned int height;

    /* parent */
    window_t *parent;
    unsigned int x;
    unsigned int y;

    /* scroll info */
    unsigned int scroll_y;
    unsigned int show_scroll;

    /* icons */
    struct {
        pixbuf_t *pixbuf;
        char title[30];
        int x;
        int y;
        int tx;
        int ty;
    } icons[500];

    /* selected icon */
    int selected;

    /* events */
    void (*double_click)(int selected);

    /* the pixbuf */
    pixbuf_t *pixbuf;

    /* the active part (the part that appears on the parent window) */
    pixbuf_t *active;

} iconview_t;

iconview_t *iconview_alloc(unsigned int count,
                           unsigned int icon_width,
                           unsigned int icon_height,
                           unsigned int width,
                           unsigned int height,
                           unsigned int show_scroll);
void iconview_insert(iconview_t *iv, pixbuf_t *icon, char *title);

#endif
