/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Core Utilities.                             | |
 *        | |  -> cat.                                             | |
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

int main(int argc, char *argv[], char *envp[]) {

    char c; /* used as buffer. */
    int ret = 0;

    if (argc == 1) {
        /* read from stdin and print to stdout */
        while(scanf("%c", &c) != EOF)
            printf("%c", c);
    } else {
        /* input is specified by arguments. */
        int i;
        for (i = 1; i < argc; i++) {
            /* open the file: */
            FILE *f = fopen(argv[i], "r");
            if (!f) {
                fprintf(stderr, "cat: can't open %s\n", argv[i]);
                ret = -1;
                continue;
            }

            /* read from the file and print to stdout */
            while(fscanf(f, "%c", &c) != EOF)
                printf("%c", c);

            /* close the file */
            fclose(f);
        }
    }

    return ret;

}
