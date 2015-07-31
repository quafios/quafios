/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Core Utilities.                             | |
 *        | |  -> cp.                                              | |
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

int main(int argc, char *argv[], char *envp[]) {

    char c; /* used as buffer. */
    int ret = 0;
    FILE *in, *out;

    /* make sure arguments are valid */
    if (argc != 3) {
        fprintf(stderr, "Invalid arguments!\n");
        return -1;
    }

    /* open input file for read */
    in = fopen(argv[1], "r");
    if (!in) {
        fprintf(stderr, "cat: can't open %s\n", argv[1]);
        return -1;
    }

    /* open output file for write */
    out = fopen(argv[2], "w");
    if (!out) {
        fprintf(stderr, "cat: can't open %s\n", argv[2]);
        fclose(in);
        return -1;
    }

    /* read from the input file and print to output file */
    while(fscanf(in, "%c", &c) != EOF)
        fprintf(out, "%c", c);

    /* close the files */
    fclose(in);
    fclose(out);

    /* done */
    return ret;

}
