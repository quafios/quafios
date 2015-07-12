/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Shell.                                      | |
 *        | |  -> I/O operations.                                  | |
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
#include <sys/fs.h>
#include <video/generic.h>
#include <tty/vtty.h>

#include "io.h"

void setAttr(unsigned char data) {
    ioctl(1 /*stdout*/, TTY_ATTR, (void *) &data);
}

void setAttrAtOff(unsigned char x, unsigned char y, unsigned char color) {
    unsigned char data[3];
    data[0] = x;
    data[1] = y;
    data[2] = color;
    ioctl(1 /*stdout*/, TTY_SETATTRATOFF, (void *) data);
}

unsigned char getCurLine() {
    unsigned char data[2];
    ioctl(1 /*stdout*/, TTY_GETCURSOR, (void *) data);
    return data[1]; /* return y offset. */
}

void setLine(unsigned char color) {

    int y = getCurLine();
    int x;

    for(x = 0; x < CONSOLE_WIDTH; x++)
        setAttrAtOff(x, y, color);

}

void prompt() {

    /* draw the prompt's line: */
    setLine(PROMPT_LINE_COLOR);
    setAttr(PROMPT_COLOR);
    printf("$ ");
    setAttr(PROMPT_LINE_COLOR);

}

void color() {

    /* initialize the console for next job: */
    setLine(PROG_COLOR);
    setAttr(PROG_COLOR);

}

void readcmd(char *cmd) {
    gets(cmd);
}
