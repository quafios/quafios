/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios GUI Library.                                | |
 *        | |  -> Alex graphics library.                           | |
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

#include <gui.h>

FILE *fnt;
char font[256][16];
int font_loaded = 0;
static int font_width;
static int font_height;

void draw_solid(pixbuf_t *pixbuf, int x, int y,
                int width, int height, int color) {

    int dx, dy;

    for (dy = y; dy < y+height; dy++)
        for (dx = x; dx < x+width; dx++)
            pixbuf_set_pixel(pixbuf, dx, dy, color);

}

void use_font(unsigned int width, unsigned int height) {

    int i;

    if (width == 6 && height == 12) {

        fnt = fopen("/usr/share/fonts/font6x12.fon", "r");
        for (i = 0; i < 256; i++)
            fread(font[i], height, 1, fnt);
        fclose(fnt);
        font_width = 6;
        font_height = 12;

    } else {

        fnt = fopen("/usr/share/fonts/font8x16.fon", "r");
        fread(font, sizeof(font), 1, fnt);
        fclose(fnt);
        font_width = 8;
        font_height = 16;

    }

    font_loaded = 1;

}

void draw_text(pixbuf_t *pixbuf,
               unsigned int x,
               unsigned int y,
               char *str,
               unsigned int color) {

    int i = 0;

    if (font_loaded == 0) {
        use_font(8, 16);
    }

    while (str[i]) {

        unsigned int j, k;
        unsigned char c = str[i++];

        for (j = 0; j < font_width; j++)
            for (k = 0; k < font_height; k++) {
                if (font[c][k]&(1<<(7-j))) {
                    pixbuf_set_pixel(pixbuf, x+j, y+k, color);
                }
            }

        x += font_width+1;

    }

}



void draw_text_bg(pixbuf_t *pixbuf,
               unsigned int x,
               unsigned int y,
               char *str,
               unsigned int color,
               unsigned int bg) {

    int i = 0;

    if (font_loaded == 0) {
        use_font(8, 16);
    }

    while (str[i]) {

        unsigned int j, k;
        unsigned char c = str[i++];

        for (j = 0; j <= font_width; j++)
            for (k = 0; k < font_height; k++) {
                if (j < font_width && font[c][k]&(1<<(7-j))) {
                    pixbuf_set_pixel(pixbuf, x+j, y+k, color);
                } else {
                    pixbuf_set_pixel(pixbuf, x+j, y+k, bg);
                }
            }

        x += font_width+1;

    }
}
