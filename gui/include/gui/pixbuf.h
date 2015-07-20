/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios GUI Library.                                | |
 *        | |  -> Pixel buffer header.                             | |
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

typedef struct pixbuf {

    unsigned int width;
    unsigned int height;
    unsigned int *buf; /* pixels */

} __attribute__((packed)) pixbuf_t;

pixbuf_t *pixbuf_alloc(unsigned int width, unsigned int height);

void pixbuf_set_pixel(pixbuf_t *pixbuf,
                      unsigned int x,
                      unsigned int y,
                      unsigned int rgb);

unsigned int pixbuf_get_pixel(pixbuf_t *pixbuf,
                              unsigned int x,
                              unsigned int y);

void pixbuf_crop(pixbuf_t *src, pixbuf_t *dest,
                 unsigned int x, unsigned int y);

void pixbuf_paint(pixbuf_t *src, pixbuf_t *dest,
                  unsigned int x, unsigned int y);
