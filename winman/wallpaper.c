/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Window Manager.                             | |
 *        | |  -> Wallpaper routines.                              | |
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

#include "winman.h"

int which = -1;

int color = 0;

/* color:
    wallpaper = pixbuf_alloc(osm->width, osm->height);
    for (i = 0; i < osm->width*osm->height; i++)
        wallpaper->buf[i] = 0xFF008080;
 */

pixbuf_t *wallpaper[6] = {0};

char *filename[6] = {
    "/usr/share/wallpapers/clouds.png",
    "/usr/share/wallpapers/sky.png",
    "/usr/share/wallpapers/rainbow.png",
    "/usr/share/wallpapers/london.png",
    "/usr/share/wallpapers/turing.png",
    "/usr/share/wallpapers/alex.png"
};

void set_wallpaper(int index) {

    extern int chunk_count;

    /* store the index on the disk */
    FILE *fd = fopen("/run/wallpaper", "w");
    fprintf(fd, "%d\n", index);
    fclose(fd);

    /* deallocate current buffer */
    if (which >= 0 && wallpaper[which]) {
        free(wallpaper[which]->buf);
        free(wallpaper[which]);
    }

    /* read the png file */
    wallpaper[index] = parse_png(filename[index]);

    /* store the index */
    which = index;

}

void draw_wallpaper() {

    int i;
    pixbuf_t *osm = get_osm();

    if (which == -1)
        set_wallpaper(0);

    for (i = 0; i < osm->width*osm->height; i++)
        osm->buf[i] = wallpaper[which]->buf[i];

}
