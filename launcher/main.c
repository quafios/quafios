/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Launcher.                                   | |
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

void launch(int selected) {

    if (selected == 0) {
        /* qonqueror */
        if (!fork())
            execve("/bin/qonqueror", NULL, NULL);
    } else if (selected == 2) {
        /* qonsole */
        if (!fork())
            execve("/bin/qonsole", NULL, NULL);
    } else if (selected == 3) {
        /* control panel */
        if (!fork())
            execve("/bin/cpanel", NULL, NULL);
    }
}

int main() {

    pixbuf_t *icon1 = parse_png("/usr/share/icons/browser48.png");
    pixbuf_t *icon2 = parse_png("/usr/share/icons/edit48.png");
    pixbuf_t *icon3 = parse_png("/usr/share/icons/qonsole48.png");
    pixbuf_t *icon4 = parse_png("/usr/share/icons/cpanel48.png");
    pixbuf_t *icon5 = parse_png("/usr/share/icons/poweroff48.png");
    window_t *win;
    iconview_t *iconview;

    /* allocate a window */
    win = window_alloc("Quafios Launcher", /* title */
                       465,                /* width */
                       100,                /* height */
                       -2,                 /* x (center) */
                       -2,                 /* y (center) */
                       0xFFC0C0C0,         /* bg color */
                       "/usr/share/icons/launcher16.png" /* iconfile */);

    /* allocate iconview */
    iconview = iconview_alloc(5, 48, 48,
                              win->pixbuf->width,
                              win->pixbuf->height, 0);

    /* insert iconview */
    window_insert(win, iconview, 0, 0);

    /* insert icons */
    iconview_insert(iconview, icon1, "Home");
    iconview_insert(iconview, icon2, "Editor");
    iconview_insert(iconview, icon3, "Qonsole");
    iconview_insert(iconview, icon4, "Settings");
    iconview_insert(iconview, icon5, "Turn off");

    /* initialize events */
    iconview->double_click = launch;

    /* redraw on the window */
    iconview_redraw(iconview);

    /* loop */
    gui_loop();

    /* done */
    return 0;

}
