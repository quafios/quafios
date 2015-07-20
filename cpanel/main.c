/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Control Panel.                              | |
 *        | |  -> main() procedure.                                | |
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

#include <stdio.h>
#include <gui.h>
#include <api/proc.h>

window_t *win;

int count = 6;

int selected = -1;

char *str[] = {
    "Clouds.",
    "Sky.",
    "Proud rainbow, proud nature.",
    "London",
    "Alan Turing",
    "My city :)"
};

void draw_radio(int index, char *text, int selected) {

    int y = 30+20*index;

    draw_solid(win->pixbuf, 10, y, 16, 16, 0xFF000000);
    draw_solid(win->pixbuf, 11, y+1, 14, 14, 0xFFFFFFFF);

    if (selected)
        draw_solid(win->pixbuf, 13, y+3, 10, 10, 0xFF000000);

    draw_text(win->pixbuf, 35, y, text, 0xFF000000);

}

void select(int index) {

    wm_set_wallpaper_t req;

    if (selected != -1)
        draw_radio(selected, str[selected], 0);
    selected = index;
    draw_radio(selected, str[selected], 1);
    window_flush(win, 0, 0, win->pixbuf->width, win->pixbuf->height);

    req.prefix = 0;
    req.type   = WMREQ_SET_WP;
    req.id     = 0;
    req.index  = index;
    wm_req(&req, sizeof(wm_set_wallpaper_t));

}

void clicked(unsigned int x, unsigned int y) {

    int i;
    for (i = 0; i < count; i++) {
        int rx = 10;
        int ry = 30+20*i;
        int width = 16;
        int height = 16;
        if (x >= rx && y >= ry && x < rx+width && y < ry+height) {
            select(i);
            return;
        }
    }

}

int get_wallpaper() {

    char x;
    FILE *fd = fopen("/run/wallpaper", "r");
    fscanf(fd, "%c", &x);
    fclose(fd);
    return x-'0';

}

int main() {

    int i;

    /* allocate a window */
    win = window_alloc("Control Panel", /* title      */
                       300,         /* width      */
                       160,         /* height     */
                       -2,          /* x (center) */
                       -2,          /* y (center) */
                       0xFFFFE0E0,  /* bg color   */
                       "/usr/share/icons/cpanel16.png" /* iconfile */);

    /* initialize event handlers */
    win->click = clicked;

    /* print header */
    draw_text(win->pixbuf, 10, 10, "Select a wallpaper :D", 0xFF000000);

    /* add radio buttons */
    for (i = 0; i < count; i++)
        draw_radio(i, str[i], 0);

    /* get current wallpaper */
    select(get_wallpaper());

    /* loop */
    gui_loop();

    /* done */
    return 0;

}
