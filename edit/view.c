/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Text Editor.                                | |
 *        | |  -> View mode operations.                            | |
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

#include <stdio.h>

void view_mode() {
    console_title("View mode");
    console_status("Press ESC to exit.");
    console_update();
    while(1) {
        char c = getchar();
        switch (c) {
            case 0: /* nothing */
            break;

            case 18: /* home */
            break;

            case 19: /* end */
            break;

            case 20: /* page up */
            console_pageup();
            break;

            case 21: /* page down */
            console_pagedown();
            break;

            case 23: /* up */
            console_up();
            break;

            case 24: /* down */
            console_down();
            break;

            case 25: /* right */
            console_right();
            break;

            case 26: /* left */
            console_left();
            break;

            case 27: /* ESC */
            return;

        }
    }
}
