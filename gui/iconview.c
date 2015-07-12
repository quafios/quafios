/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios GUI Library.                                | |
 *        | |  -> Iconview element.                                | |
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
#include <stdlib.h>
#include <api/proc.h>
#include <api/mman.h>

static pixbuf_t *up_png = NULL;
static pixbuf_t *down_png = NULL;

iconview_t *iconview_alloc(unsigned int count,
                           unsigned int icon_width,
                           unsigned int icon_height,
                           unsigned int width,
                           unsigned int height,
                           unsigned int show_scroll) {

    /* allocate an iconview element */
    int i;
    iconview_t *ret    = malloc(sizeof(iconview_t));
    ret->type          = 0;
    ret->count         = count;
    ret->cur           = 0;
    ret->icon_width    = icon_width;
    ret->icon_height   = icon_height;
    ret->each_width    = icon_width  + 40;
    ret->each_height   = icon_height + 30;
    ret->icons_per_row = (width-20 /* margin * 2 */)/(ret->each_width);
    ret->rows          = count/ret->icons_per_row +
                         (count%ret->icons_per_row?1:0);
    ret->width         = width;
    ret->height        = height;
    ret->parent        = NULL;
    ret->x             = 0;
    ret->y             = 0;
    ret->scroll_y      = 0;
    ret->show_scroll   = show_scroll;
    ret->pixbuf        = pixbuf_alloc(width,
                                      height>ret->rows*ret->each_height?height:
                                      ret->rows*ret->each_height);
    ret->selected      = -1;
    ret->double_click  = NULL;
    ret->active        = NULL;
    for (i = 0; i < ret->pixbuf->width * height; i++)
        ret->pixbuf->buf[i] = 0xFFFFFFFF;
    return ret;

}

void iconview_reset(iconview_t *iv) {

    int i;

    iv->cur      = 0;
    iv->selected = -1;
    iv->scroll_y = 0;

    for (i = 0; i < iv->pixbuf->width * iv->height; i++)
        iv->pixbuf->buf[i] = 0xFFFFFFFF;

}

void iconview_insert(iconview_t *iv, pixbuf_t *icon, char *title) {

    int row = iv->cur/iv->icons_per_row;
    int y   = row * iv->each_height;
    int x   = iv->cur%iv->icons_per_row * iv->each_width + 10 /* margin */;
    int tx  = x + (iv->each_width-(strlen(title)*8))/2;
    int ty  = y + 65;

    if (!(iv->cur%iv->icons_per_row)) {
        /* new row! */
        int x, y;
        for (y = row*iv->each_height; y < (row+1)*iv->each_height; y++)
            for (x = 0; x < iv->pixbuf->width; x++)
                pixbuf_set_pixel(iv->pixbuf, x, y, 0xFFFFFFFF);
    }

    iv->icons[iv->cur].x = x;
    iv->icons[iv->cur].y = y;
    iv->icons[iv->cur].tx = tx;
    iv->icons[iv->cur].ty = ty;
    iv->icons[iv->cur].pixbuf = icon;
    strcpy(iv->icons[iv->cur].title, title);
    iv->cur++;
    pixbuf_paint(icon, iv->pixbuf, x+20, y+15);
    draw_text(iv->pixbuf, tx, ty, title, 0xFF000000);

}

void iconview_redraw(iconview_t *iv) {

    int sb_x, sb_y, sb_width, sb_height;
    int sr_x, sr_y, sr_width, sr_height;
    int rows, iheight, i;

    if (!iv->parent)
        return;

    /* clear the tmp buf */
    for (i = 0; i < iv->active->width * iv->active->height; i++)
        iv->active->buf[i] = 0xFFFFFFFF;

    /* clear the parent window (we do this because the iv contains some
     * transparent pixels that needs white background.
     */
    pixbuf_paint(iv->active, iv->parent->pixbuf, iv->x, iv->y);

    /* draw the active part of the iconview to the tmp buf */
    pixbuf_crop(iv->pixbuf, iv->active, 0, iv->scroll_y);

    /* draw the scroll bar to the tmp buf */
    if (!up_png) {
        up_png   = parse_png("/usr/share/icons/up.png");
        down_png = parse_png("/usr/share/icons/down.png");
    }

    if (iv->show_scroll) {

        pixbuf_paint(up_png, iv->active, iv->width-up_png->width, 0);
        pixbuf_paint(down_png, iv->active, iv->width-down_png->width,
                                        iv->height-down_png->height);

        sb_x = iv->width-up_png->width;
        sb_y = up_png->height;
        sb_width = up_png->width;
        sb_height = iv->height-up_png->height-down_png->height;
        draw_solid(iv->active, sb_x, sb_y, sb_width, sb_height, 0xFFFF9494);

        rows = iv->cur/iv->icons_per_row + (iv->cur%iv->icons_per_row?1:0);
        iheight = iv->height > rows*iv->each_height?
                iv->height : rows*iv->each_height;

        sr_x = sb_x + 2;
        sr_y = sb_y + (iv->scroll_y*sb_height)/iheight;
        sr_width = sb_width-4;
        sr_height = (iv->height*sb_height)/iheight;
        draw_solid(iv->active, sr_x, sr_y, sr_width, sr_height, 0xFFA93131);

    }

    /* draw the tmp buf on the parent window */
    pixbuf_paint(iv->active, iv->parent->pixbuf, iv->x, iv->y);

    /* flush the window buffer to VGA */
    window_flush(iv->parent, iv->x, iv->y, iv->width, iv->height);

}

void iconview_click(iconview_t *iv, int x, int y) {

    int icon = 0, i;
    pixbuf_t *pbuf;

    if (up_png && iv->show_scroll && x >= iv->width-up_png->width) {

        /* scroll bar area */
        int rows, iheight;
        rows = iv->cur/iv->icons_per_row + (iv->cur%iv->icons_per_row?1:0);
        iheight = iv->height > rows*iv->each_height?
                  iv->height : rows*iv->each_height;

        if (y < up_png->height) {
            /* up 10 points */
            iv->scroll_y -= 50;
            if (((int)iv->scroll_y) < 0)
                iv->scroll_y = 0;
        } else if (y > iv->height-down_png->height) {
            /* down 10 points */
            iv->scroll_y += 50;
            if (iv->scroll_y + iv->height >= iheight)
                iv->scroll_y = iheight - iv->height;

        } else {
            /* scroller clicked */

        }

        iconview_redraw(iv);
        return;

    }

    for (icon = 0; icon < iv->cur; icon++) {
        if (x >= iv->icons[icon].x &&
            iv->scroll_y+y >= iv->icons[icon].y &&
            x < iv->icons[icon].x + iv->each_width &&
            iv->scroll_y+y < iv->icons[icon].y + iv->each_height)
        break;
    }

    /* unselect previous element */
    if (iv->selected != -1) {
        int i, icon=iv->selected;
        pixbuf_t *pbuf = iv->icons[icon].pixbuf;
        for (i = 0; i < pbuf->width*pbuf->height; i++) {
            int x = iv->icons[icon].x + i%pbuf->height + 20;
            int y = iv->icons[icon].y + i/pbuf->height + 15;
            int color = pbuf->buf[i];
            pixbuf_set_pixel(iv->pixbuf, x, y, alpha(color, 0xFFFFFFFF));
        }
        draw_text_bg(iv->pixbuf, iv->icons[icon].tx, iv->icons[icon].ty,
                     iv->icons[icon].title, 0xFF000000, 0xFFFFFFFF);
    }

    /* something to be selected? */
    if (icon == iv->cur) {
        iv->selected = -1;
        iconview_redraw(iv);
        return;
    }

    /* select it! */
    iv->selected = icon;
    pbuf = iv->icons[icon].pixbuf;
    for (i = 0; i < pbuf->width*pbuf->height; i++) {

        int x = iv->icons[icon].x + i%pbuf->height + 20;
        int y = iv->icons[icon].y + i/pbuf->height + 15;

        int b = ((pbuf->buf[i]>> 0)&0xFF);
        int g = ((pbuf->buf[i]>> 8)&0xFF);
        int r = ((pbuf->buf[i]>>16)&0xFF);
        int a = ((pbuf->buf[i]>>24)&0xFF);

        int color = (a<<24) | ((r/3)<<16) | ((g/3)<< 8) | (0xBF<< 0);

        pixbuf_set_pixel(iv->pixbuf, x, y, color);
        pixbuf_set_pixel(iv->parent->pixbuf, iv->x+x, iv->y+y, 0xFFFFFFFF);

    }

    /* draw text with blue background */
    draw_text_bg(iv->pixbuf, iv->icons[icon].tx, iv->icons[icon].ty,
                 iv->icons[icon].title, 0xFFFFFFFF, 0xFF0000BF);

    iconview_redraw(iv);

}

void iconview_double(iconview_t *iv, int x, int y) {

    if (iv->selected != -1 && iv->double_click)
        iv->double_click(iv->selected);
}
