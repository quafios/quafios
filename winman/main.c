/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Window Manager.                             | |
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

#include "winman.h"

int main(int argc, char *argv[]) {

    /* initialize vga */
    vga_init();

    /* initialize keyboard */
    keyboard_init();

    /* initialize mouse */
    mouse_init();

    /* initialize osm */
    osm_init();

    /* initialize winpool */
    winpool_init();

    /* initialize inbox */
    inbox_init();

    /* draw wallpaper on the OSM */
    draw_wallpaper();

    /* draw osm to vga */
    vga_plot(get_osm(), 0, 0);

    /* draw the mouse cursor */
    draw_cursor();

    /* load launcher */
    if (!fork())
        execve("/bin/launcher", NULL, NULL);

    /* enter the inbox loop */
    inbox_listen();

}
