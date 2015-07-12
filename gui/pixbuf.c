/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Window Manager.                             | |
 *        | |  -> Pixel buffer routines.                           | |
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

#include <stdlib.h>
#include <gui.h>
#include <api/mman.h>

pixbuf_t *pixbuf_alloc(unsigned int width, unsigned int height) {

    /* allocate a new pixbuf_t */
    pixbuf_t *pixbuf = malloc(sizeof(pixbuf_t));
    pixbuf->width  = width;
    pixbuf->height = height;
    pixbuf->buf = malloc(pixbuf->width*pixbuf->height*sizeof(unsigned int));
    return pixbuf;

}

void pixbuf_set_pixel(pixbuf_t *pixbuf,
                      unsigned int x,
                      unsigned int y,
                      unsigned int color) {

    /* set the color of some pixel in the pixel buffer */
    if (x < pixbuf->width && y < pixbuf->height)
        pixbuf->buf[y*pixbuf->width + x] = color;

}

unsigned int pixbuf_get_pixel(pixbuf_t *pixbuf,
                              unsigned int x,
                              unsigned int y) {

    /* get the color of some pixel in the pixel buffer */
    if (x < pixbuf->width && y < pixbuf->height)
        return pixbuf->buf[y*pixbuf->width + x];
    else
        return 0;

}

void pixbuf_crop(pixbuf_t *src, pixbuf_t *dest,
                 unsigned int x, unsigned int y) {

    /* copy part of src into dest */
    int dx, dy;
    for (dy = 0; dy < dest->height; dy++)
        for (dx = 0; dx < dest->width; dx++)
            pixbuf_set_pixel(dest, dx, dy, pixbuf_get_pixel(src,x+dx,y+dy));

}

unsigned int alpha(unsigned int src_color, unsigned int dest_color) {

    /* extract the rgb of the source color */
    unsigned int src_b = (src_color>> 0) & 0xFF;
    unsigned int src_g = (src_color>> 8) & 0xFF;
    unsigned int src_r = (src_color>>16) & 0xFF;

    /* extract the rgb of the dest color */
    unsigned int dest_b = (dest_color>> 0) & 0xFF;
    unsigned int dest_g = (dest_color>> 8) & 0xFF;
    unsigned int dest_r = (dest_color>>16) & 0xFF;

    /* extract src_alpha (and assume dest_alpha = 1) */
    unsigned int alpha = (src_color>>24) & 0xFF;

    /* out_rgb = src_rgb * src_a + dst_rgb(1 - src_a) */
    dest_b = (src_b*alpha + dest_b*(255-alpha))/255;
    dest_g = (src_g*alpha + dest_g*(255-alpha))/255;
    dest_r = (src_r*alpha + dest_r*(255-alpha))/255;

    /* reconstruct dest_color */
    dest_color = (dest_b<<0)+(dest_g<<8)+(dest_r<<16)+(0xFF<<24);

    /* return */
    return dest_color;

#if 0
    /* extract the rgb of the source color */
    double src_b = ((double)((src_color>> 0)&0xFF))/((double)255);
    double src_g = ((double)((src_color>> 8)&0xFF))/((double)255);
    double src_r = ((double)((src_color>>16)&0xFF))/((double)255);
    double src_a = ((double)((src_color>>24)&0xFF))/((double)255);

    /* extract the rgb of the dest color */
    double dst_b = ((double)((src_color>> 0)&0xFF))/((double)255);
    double dst_g = ((double)((src_color>> 8)&0xFF))/((double)255);
    double dst_r = ((double)((src_color>>16)&0xFF))/((double)255);
    double dst_a = ((double)((src_color>>24)&0xFF))/((double)255);

    /* apply the formula */
    double out_a = src_a + dst_a*(1-src_a);
    double out_b = (src_b*src_a+dst_b*dst_a*(1-src_a))/out_a;
    double out_g = (src_g*src_a+dst_g*dst_a*(1-src_a))/out_a;
    double out_r = (src_r*src_a+dst_r*dst_a*(1-src_a))/out_a;

    /* build the new color */
    int color = (((int)(out_a*255))<<24) |
                (((int)(out_r*255))<<16) |
                (((int)(out_g*255))<< 8) |
                (((int)(out_b*255))<< 0);

    /* done */
    return color;
#endif
}

void pixbuf_paint(pixbuf_t *src, pixbuf_t *dest,
                  unsigned int x, unsigned int y) {

    int dx, dy;
    for (dy = 0; dy < src->height; dy++)
        for (dx = 0; dx < src->width; dx++) {

            unsigned int src_color  = pixbuf_get_pixel(src, dx, dy);
            unsigned int dest_color = pixbuf_get_pixel(dest, x+dx, y+dy);
            pixbuf_set_pixel(dest, x+dx, y+dy, alpha(src_color, dest_color));

        }

}
