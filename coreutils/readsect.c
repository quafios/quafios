/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Core Utilities.                             | |
 *        | |  -> readsect.c                                       | |
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
#include <errno.h>
#include <stdlib.h>
#include <api/fs.h>

int main(int argc, char *argv[], char *envp[]) {

    int fd;
    char buf[512];
    int i;

    if (argc != 3) {
        printf("Error: invalid arguments.\n");
        return -1;
    }

    /* open file */
    fd = open(argv[1], 0);

    /* seek to sector */
    seek(fd, atoi(argv[2])*512, SEEK_CUR);

    /* read */
    read(fd, buf, 512);

    /* print */
    for (i = 0; i < 512; i++)
        printf("%c", buf[i]);
    printf("\n");

    /* done */
    close(fd);
    return 0;
}

