/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Console.                                    | |
 *        | |  -> main() procedure.                                | |
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
#include <string.h>
#include <gui.h>
#include <api/proc.h>
#include <api/fs.h>
#include <tty/vtty.h>
#include <tty/pstty.h>
#include <timer/generic.h>

int timer_fd;
int scheduled = 0;

int pstty_fd;
char pstty_file[20] = "/dev/pstty";

window_t *win;

static unsigned char font[256][16];
unsigned int font_width;
unsigned int font_height;

int x1 = -1, y1 = -1, x2 = -1, y2 = -1;

void font_init() {

    int i, j;
    FILE *fnt = fopen("/usr/share/fonts/font6x12.fon", "r");
    font_width = 6;
    font_height = 12;

    for (i = 0; i < 256; i++)
        for (j = 0; j < font_height; j++)
            fscanf(fnt, "%c", &font[i][j]);

    fclose(fnt);

}

void pstty_init() {

    int tty_fd;
    tty_fork_t tty_fork = {0};

    /* open vtty driver */
    tty_fd = open("/dev/console", 0);

    /* send tty_fork command */
    tty_fork.prefix = 0x10;
    ioctl(tty_fd, TTY_FORK, &tty_fork);

    /* close tty */
    close(tty_fd);

    /* create pstty device file */
    itoa(getpid(), &pstty_file[strlen(pstty_file)], 10);
    mknod(pstty_file, 0x2777, tty_fork.devid);

    /* open the pstty */
    pstty_fd = open(pstty_file, 0);

}

void timer_init() {

    /* open vtty driver */
    timer_fd = open("/dev/timer", 0);

}

void exec_shell(char *fname) {

    /* fork */
    if (!fork()) {

        /* close standard streams */
        close(0);
        close(1);
        close(2);

        /* change to /home */
        chdir("/home");

        /* open pstty as the standard I/O stream */
        open(pstty_file, 0); /* open pstty_file as stdin */
        open(pstty_file, 0); /* open pstty_file as stdout */
        open(pstty_file, 0); /* open pstty_file as stderr */

        /* execute the shell */
        execve(fname, NULL, NULL);

    }

}

/***************************************************************************/
/*                        Pseudo text-mode VGA                             */
/***************************************************************************/

#define VGA_MAX_COLS    80
#define VGA_MAX_ROWS    25

char buf[80*25*2] = {0};
static uint32_t vga_row = 0, vga_col = 0;
static uint32_t vga_attrib = 0x0F;
static uint32_t color_palette[] = {
     0xFF000000,
     0xFF0000AA,
     0xFF00AA00,
     0xFF00AAAA,
     0xFFAA0000,
     0xFFAA00AA,
     0xFFAA5500,
     0xFFAAAAAA,
     0xFF555555,
     0xFF5555FF,
     0xFF55FF55,
     0xFF55FFFF,
     0xFFFF5555,
     0xFFFF55FF,
     0xFFFFFF55,
     0xFFFFFFFF
};

void draw_char(char chr, char attr, int x, int y) {

    unsigned int chr_pos_x = (font_width)*x;
    unsigned int chr_pos_y = (font_height)*y;
    unsigned int bg = color_palette[(attr >> 4) & 0xF];
    unsigned int fg = color_palette[(attr >> 0) & 0xF];
    char str[2];
    int i, j, k;

    /* write to text buffer */
    buf[y*VGA_MAX_COLS*2+x*2+0] = chr;
    buf[y*VGA_MAX_COLS*2+x*2+1] = attr;

    /* draw using the font */
    for (j = 0; j < font_width; j++)
        for (k = 0; k < font_height; k++)
            if (font[chr][k]&(1<<(7-j)))
                pixbuf_set_pixel(win->pixbuf, chr_pos_x+j, chr_pos_y+k, fg);
            else
                pixbuf_set_pixel(win->pixbuf, chr_pos_x+j, chr_pos_y+k, bg);

    /* update watchers */
    if (x1 == -1) {
        x1 = chr_pos_x;
        y1 = chr_pos_y;
        x2 = chr_pos_x + font_width;
        y2 = chr_pos_y + font_height;
    } else {

        if (chr_pos_x < x1)
            x1 = chr_pos_x;

         if (chr_pos_y < y1)
            y1 = chr_pos_y;

        if (chr_pos_x + font_width > x2)
            x2 = chr_pos_x + font_width;

        if (chr_pos_y + font_height > y2)
            y2 = chr_pos_y + font_height;

    }

    /* flush */
    if (!scheduled) {
        timer_alert_t alert;
        alert.prefix = 0x11;
        alert.time = 10;
        ioctl(timer_fd, TIMER_ALERT, &alert);
        scheduled = 1;
    }

}

void scroll() {
    unsigned int i, j;

    for (i=0; i< 80 * 24 * 2; i++)
        buf[i] = buf[i + 80 * 2];

    /* clean last row: */
    for (; i < 80 * 25 * 2; i+=2)
        buf[i] = ' ';

    /* redraw all lines */
    for (i = 0; i < VGA_MAX_ROWS; i++)
        for (j = 0; j < VGA_MAX_COLS; j++)
            draw_char(buf[i*VGA_MAX_COLS*2+j*2+0],
                      buf[i*VGA_MAX_COLS*2+j*2+1],j,i);
}

void newline() {

    vga_col = 0;

    if (vga_row != VGA_MAX_ROWS - 1) {
        vga_row++;
    } else {
        scroll();
    }

    /* TODO: update_cursor(); */
}

void pstty_putc(char chr) {

    char new_col = 0, new_row = 0;

    switch (chr) {
        case '\n':
            newline();
            break;

        case '\t':
            do pstty_putc(' '); while(vga_col%8);
            break;

        case '\b':
            if (vga_col == 0 && vga_row) {
                vga_row--;
                vga_col = VGA_MAX_COLS - 1;
            } else if (vga_row == 0 && vga_col == 0) {
                /* Do nothing... */
            } else {
                vga_col = vga_col - 1;
            }
            /* TODO: update_cursor(); */
            break;

        default:

            draw_char(chr, vga_attrib, vga_col, vga_row);

            if (vga_col != VGA_MAX_COLS-1) {
                vga_col = vga_col+1;
                /* TODO: update_cursor(); */
            } else {
                newline();
            }
    }

}

void pstty_change_attr(char attr) {
    vga_attrib = attr;
}

void pstty_set_attr_at_off(char x, char y, char attr) {
    draw_char(buf[y*VGA_MAX_COLS*2+x*2], attr, x, y);
}

void pstty_set_char_at_off(char x, char y, char c) {
    draw_char(c, buf[y*VGA_MAX_COLS*2+x*2+1], x, y);
}

void pstty_get_cursor() {
    char cursor[2];
    cursor[0] = vga_col;
    cursor[1] = vga_row;
    ioctl(pstty_fd, TTY_SETCURSOR, cursor);
}

void pstty_set_cursor(char x, char y) {
    vga_row = y;
    vga_col = x;
}

int k = 0;

void pstty_event(void *packet) {

    /* extract data from the packet */
    char cmd  = ((pstty_packet_t *) packet)->cmd;
    char chr  = ((pstty_packet_t *) packet)->chr;
    char attr = ((pstty_packet_t *) packet)->attr;
    char x    = ((pstty_packet_t *) packet)->x;
    char y    = ((pstty_packet_t *) packet)->y;

    if (((pstty_packet_t *) packet)->prefix == 0x11) {
        /* timer alert */
        window_flush(win, x1, y1, x2-x1, y2-y1);
        x1 = x2 = y1 = y2 = -1;
        scheduled = 0;
        return;
    }

    /* process the cmd */
    switch (cmd) {

        case PSTTY_PUTC:
            pstty_putc(chr);
            break;

        case PSTTY_CHANGE_ATTR:
            pstty_change_attr(attr);
            break;

        case PSTTY_SET_ATTR_AT_OFF:
            pstty_set_attr_at_off(x, y, attr);
            break;

        case PSTTY_SET_CHAR_AT_OFF:
            pstty_set_char_at_off(x, y, chr);
            break;

        case PSTTY_SET_CURSOR:
            pstty_set_cursor(x, y);
            break;

        case PSTTY_GET_CURSOR:
            pstty_get_cursor();
            break;

        default:
            break;

    }

}

void pstty_press(char chr) {

    ioctl(pstty_fd, TTY_PRESS, &chr);

}

int main() {

    /* initialize the font */
    font_init();

    /* initialize timer */
    timer_init();

    /* allocate a window */
    win = window_alloc("Qonsole", /* title */
                       font_width*80, /* width */
                       font_height*25, /* height */
                       -1,             /* x (random) */
                       -1,             /* y (random) */
                       0xFF000000, /* bg color */
                       "/usr/share/icons/qonsole16.png" /* iconfile */);

    /* initialize window event handlers */
    win->press = pstty_press;

    /* initialize pstty */
    pstty_init();

    /* execute the shell */
    exec_shell("/bin/rash");

    /* set receiver */
    set_receiver(pstty_event);

    /* loop */
    gui_loop();

    /* done */
    return 0;

}
