/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios PNG Viewer.                                 | |
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

int main(int argc, char *argv[]) {

    window_t *win;
    pixbuf_t *pngfile;
    int win_width, win_height, win_x = 0, win_y = 0;

    /* make sure number of arguments is valid */
    if (argc < 2) {
        fprintf(stderr, "quafshot: No enough arguments.\n");
        return -1;
    }

    /* try to parse the PNG file */
    pngfile = parse_png(argv[1]);

    /* PNG file is valid? */
    if (!pngfile) {
        fprintf(stderr, "quafshot: Not a valid png file.\n");
        return -1;
    }

    /* calculate window width & height */
    if ((win_width = pngfile->width) >= 700) {
        win_width = 700;
        win_x = (win_width-((int)pngfile->width))/2;
    } else if (win_width < 200) {
        win_width = 200;
        win_x = (win_width-((int)pngfile->width))/2;
    }

    if ((win_height = pngfile->height) >= 500) {
        win_height = 500;
        win_y = (win_height-((int)pngfile->height))/2;
    }

    /* allocate a window */
    win = window_alloc("Quafshot",         /* title */
                       win_width,          /* width */
                       win_height,         /* height */
                       -2,                 /* x (center) */
                       -2,                 /* y (center) */
                       0xFFC0C0C0,         /* bg color */
                       "/usr/share/icons/launcher16.png" /* iconfile */);

    /* draw PNG on the window */
    pixbuf_paint(pngfile, win->pixbuf, win_x, win_y);

    /* flush the screen */
    window_flush(win, 0, 0, win_width, win_height);

    /* loop */
    gui_loop();

    /* done */
    return 0;

}
