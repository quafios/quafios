/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Window Manager.                             | |
 *        | |  -> BMP format support.                              | |
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
#include <stdlib.h>
#include <gui.h>

typedef struct {
    char  id[2];
    int   size;
    short reserved1;
    short reserved2;
    int   offset;
} __attribute__((packed)) bmp_file_header_t;

typedef struct {
    int size;
    int width;
    int height;
    short planes; /* number of color planes */
    short depth; /* color depth */
    int compression;
    int imgsize;    /* size of the actual image data */
    int horizontal; /* horizontal resolution */
    int vertical;   /* vertical resolution */
    int pallette;   /* number of colors in pallette */
    int important;  /* important colors */
} __attribute__((packed)) bmp_info_header_t;

pixbuf_t *parse_bmp(char *filename) {

    bmp_file_header_t header;
    bmp_info_header_t info;
    int i, j, k = 0;
    unsigned char *rgb;
    FILE *bmp = fopen(filename, "r");
    pixbuf_t *pixbuf;

    /* bmp file found? */
    if (bmp == NULL)
        return NULL;

    /* read bmp file header */
    fread(&header, sizeof(header), 1, bmp);

    /* read bmp info header */
    fread(&info, sizeof(info), 1, bmp);

    /* allocate pixbuf header */
    pixbuf = pixbuf_alloc(info.width, info.height);

    /* read pixel array */
    rgb = malloc(info.imgsize);
    fread(rgb, info.imgsize, 1, bmp);

    /* iterate on the colors */
    for (i = info.height-1; i >= 0; i--) {
        for(j = 0; j < info.width; j++) {
            pixbuf_set_pixel(pixbuf, j, i, 0xFF000000 | *((int *) &rgb[k]));
            k+= 3;
        }
        /*k+=2;*/
    }

    /* done */
    return pixbuf;

}
