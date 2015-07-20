/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Core Utilities.                             | |
 *        | |  -> mknod.                                           | |
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
#include <string.h>
#include <api/fs.h>

int isValidDevNum(char *str) {

    int i = 0;

    while(str[i]) {
        if (str[i] < '0' || str[i] > '9')
            return 0;
        i++;
    }

    if (i == 0)
        return 0;

    return 1; /* valid. */

}

int main(int argc, char *argv[]) {

    if (argc < 3 ||
        (strcmp(argv[2], "r") && strcmp(argv[2], "d")) ||
        ((!strcmp(argv[2], "r")) && argc != 3) ||
        ((!strcmp(argv[2], "d")) && argc != 4) ||
        ((!strcmp(argv[2], "d")) && (!isValidDevNum(argv[3])))) {
        fprintf(stderr,"Usage: mknod NAME r\n");
        fprintf(stderr,"       mknod NAME d DEVNUM\n");
        return -1;
    }

    if (!strcmp(argv[2], "r")) {
        /* regular file */
        if (mknod(argv[1], FT_REGULAR, 0) < 0) {
            fprintf(stderr, "Can't make %s.\n", argv[1]);
            return -1;
        }
    } else if (!strcmp(argv[2], "d")) {
        /* device file */
        if (mknod(argv[1], FT_SPECIAL, atoi(argv[3])) < 0) {
            fprintf(stderr, "Can't make %s.\n", argv[1]);
            return -1;
        }
    }

    return 0;

}
