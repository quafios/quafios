/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Text Editor.                                | |
 *        | |  -> Console organization.                            | |
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

#include <tty/vtty.h>
#include <api/fs.h>

int cols = 80;
int rows = 25;

int file_start = 0;
int win_last   = 0; /* last character in current window; */
int cursor_off = 0;
int cur_row    = 0;
int cur_col    = 0;
int seek_col   = 0; /* for moving up and down. */

char screen_copy[24][80]; /* a copy of current view. */
int screen_init = 0; /* initialized? */

extern char *fileMem;
extern int fileSize;

void setAttrAtOff(unsigned char row, unsigned char col, unsigned char color) {
    unsigned char data[3];
    data[0] = col;
    data[1] = row;
    data[2] = color;
    ioctl(1 /*stdout*/, TTY_SETATTRATOFF, (void *) data);
}

void setCharAtOff(unsigned char row, unsigned char col, unsigned char chr) {
    unsigned char data[3];
    data[0] = col;
    data[1] = row;
    data[2] = chr;
    ioctl(1 /*stdout*/, TTY_SETCHARATOFF, (void *) data);
}

void setCursor(int row, int col) {
    unsigned char data[2];
    data[0] = col;
    data[1] = row;
    ioctl(1 /*stdout*/, TTY_SETCURSOR, (void *) data);
}

unsigned char getRow() {
    unsigned char data[2];
    ioctl(1 /*stdout*/, TTY_GETCURSOR, (void *) data);
    return data[1]; /* return y offset. */
}

unsigned char getCol() {
    unsigned char data[2];
    ioctl(1 /*stdout*/, TTY_GETCURSOR, (void *) data);
    return data[0]; /* return x offset. */
}

void disable_echo() {
    ioctl(1 /*stdout*/, TTY_NOECHO, NULL);
}

void enable_echo() {
    ioctl(1 /*stdout*/, TTY_SETECHO, NULL);
}

void disable_buf() {
    ioctl(1 /*stdout*/, TTY_NOBUF, NULL);
}

void enable_buf() {
    ioctl(1 /*stdout*/, TTY_SETBUF, NULL);
}

void console_title(char *mode) {

    char *title_start = "Quafios text editor (";
    char *title_mid   = ") - ";
    char pwd[FILENAME_MAX+1] = "";
    char slash = 0;
    extern char *filename;
    int len, margin_left, margin_right;
    int i, row = 0, col = 0;

    if (filename[0] != '/') {
        getcwd(pwd, sizeof(pwd)-1);
        if (strcmp(pwd, "/"))
            slash = 1;
    }

    len = strlen(title_start) + strlen(mode) + strlen(title_mid) +
          slash + strlen(pwd) + strlen(filename);

    margin_left  = (80-len)/2;
    margin_right = 80-margin_left-len;

    for (i = 0; i < margin_left; i++)
        setCharAtOff(row, col++, ' ');

    for (i = 0; i < strlen(title_start); i++)
        setCharAtOff(row, col++, title_start[i]);

    for (i = 0; i < strlen(mode); i++)
        setCharAtOff(row, col++, mode[i]);

    for (i = 0; i < strlen(title_mid); i++)
        setCharAtOff(row, col++, title_mid[i]);

    for (i = 0; i < strlen(pwd); i++)
        setCharAtOff(row, col++, pwd[i]);

    for (i = 0; i < slash; i++)
        setCharAtOff(row, col++, '/');

    for (i = 0; i < strlen(filename); i++)
        setCharAtOff(row, col++, filename[i]);

    for (i = 0; i < margin_right; i++)
        setCharAtOff(row, col++, ' ');

}

void console_update() {

    int i, j, k = file_start, done = 0;
    char screen_tmp[24][80];

    for (i = 1; i < rows-1; i++) {
        for (j = 0; j < 80; j++) {
            screen_tmp[i][j] = 0;
        }
    }

    for (i = 1; i < rows-1; i++) {
        for (j = 0; j < 80; j++) {
            char c;
            win_last = k;
            if (k == cursor_off) {
                setCursor(i, j);
                cur_row = i;
                cur_col = j;
            }
            if (k == fileSize) {
                screen_tmp[i][j] = ' ';
                done = 1;
                break;
            }
            c = fileMem[k++];
            if (c == '\n') {
                screen_tmp[i][j] = ' ';
                break;
            }
            if (c == '\t') {
                screen_tmp[i][j] = ' ';
                j += 8-(j%8)-1;
                continue;
            }

            screen_tmp[i][j] = c;
        }
        if (done)
            break;
    }

    /* do actual update: */
    for (i = 1; i < rows-1; i++) {
        for (j = 0; j < 80; j++) {
            if ((!screen_init) ||
               (screen_tmp[i][j] != screen_copy[i][j])) {
                setCharAtOff(i, j, screen_tmp[i][j]);
                screen_copy[i][j] = screen_tmp[i][j];
            }
        }
    }
    screen_init = 1;

}

void console_scroll_down() {

    int i, j, k = file_start, next_line = 0, f = 0;

    if (win_last == fileSize)
        return;

    for (j = 0; j < 80; j++) {
        char c = fileMem[k++];
        if (c == '\n')
            break;
        if (c == '\t') {
            j += 8-(j%8)-1;
            continue;
        }
    }

    file_start = k;

    console_update();

}

void console_scroll_up() {

    int i, last_line, count;
    int prev_line = file_start;

    if (!file_start)
        return;

    /* get the beginning of previous line: */
    while(1) {
        prev_line--;
        if (!prev_line || fileMem[prev_line-1] == '\n') {
            break;
        }
    }

    /* how many columns needed for that line? */
    i = prev_line;
    last_line = 0;
    count = 0;
    do {
        if (count % 80 == 0)
            last_line = i;
        if (fileMem[i] == '\t') {
            count += 8 - (count%8);
        } else {
            count++;
        }
    } while (++i != file_start);

    /* now we know from where to start: */
    file_start = last_line;

    /* update screen: */
    console_update();

}

void console_right() {

    if (cursor_off == fileSize)
        return;

    cursor_off++;

    if (cursor_off > win_last) {
        /* need to scroll. */
        console_scroll_down();
    } else {
        /* just update: */
        console_update();
    }

    seek_col = cur_col;

}

void console_left() {

    if (cursor_off == 0)
        return;

    cursor_off--;

    /* need scroll? */
    if (cursor_off < file_start) {
        console_scroll_up();
    } else {
        /* just update: */
        console_update();
    }

    seek_col = cur_col;

}

void console_up() {

    int i, j, o;

    if (cur_row == 1) {
        /* need to scroll up! */
        console_scroll_up();
        if (cur_row == 1) /* still no change */
            return;
    }

    /* move backwards: */
    o = cursor_off;

    for (j = cur_col; j >= 0; j--) {
        if (screen_copy[cur_row][j])
            o--;
    }

    for (j = 79; j >= 0; j--) {
        if (screen_copy[cur_row-1][j]) {
            if (j <= seek_col) {
                break;
            }
            o--;
        }
    }

    cursor_off = o;

    console_update();

}

void console_down() {

    int i, j, o;

    if (cur_row == 23) {
        /* need to scroll down! */
        console_scroll_down();
        if (cur_row == 23) /* still no change */
            return;
    }

    /* move forwards: */
    o = cursor_off;

    for (j = cur_col; j < 80; j++) {
        if (screen_copy[cur_row][j])
            o++;
    }

    for (j = 0; j < 80; j++) {
        if (j >= seek_col) {
            if (!screen_copy[cur_row+1][j])
                o--;
            break;
        }
        if (screen_copy[cur_row+1][j]) {
            o++;
        }
    }

    cursor_off = o;

    console_update();

}

void console_pageup() {

    int i = 23;
    while(i--)
        console_up();

}

void console_pagedown() {

    int i = 23;
    while(i--)
        console_down();

}

void console_insert(char c) {

    /* insert a character: */
    file_insert(c, cursor_off);
    console_update();
    console_right();

}

void console_backspace() {

    /* do backspace: */
    if (!cursor_off)
        return;
    file_del(cursor_off-1);
    console_update();
    console_left();
}

void console_status(char *str) {

    int len = strlen(str);
    int margin_right = cols - len;
    int row = rows-1, col = 0;
    int i;

    for (i = 0; i < len; i++)
        setCharAtOff(row, col++, str[i]);

    setCursor(row, col);

    for (i = 0; i < margin_right; i++)
        setCharAtOff(row, col++, ' ');

}

void console_init() {
    int i, j = 0;

    for (; j < 1; j++) {
        for (i = 0; i < cols; i++) {
            setCharAtOff(j, i, ' ');
            setAttrAtOff(j, i, 0x2E);
        }
    }

    for (; j < rows-1; j++) {
        for (i = 0; i < cols; i++) {
            setCharAtOff(j, i, ' ');
            setAttrAtOff(j, i, 0x1F);
        }
    }


    for (; j < rows; j++) {
        for (i = 0; i < cols; i++) {
            setCharAtOff(j, i, ' ');
            setAttrAtOff(j, i, 0x70);
        }
    }
    disable_echo();
    disable_buf();
    console_update();
}

void console_black() {

    int i, j;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            setCharAtOff(i, j, ' ');
            setAttrAtOff(i, j, 0x0F);
        }
    }
    setCursor(0, 0);
    enable_echo();
    enable_buf();
}
