/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Text Editor.                                | |
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

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Must pass file name as argument.\n");
        return -1;
    }

    if (file_load(argv[1])) {
        printf("can't open %s\n", argv[1]);
        return -1;
    }

    if (!strcmp(&argv[0][strlen(argv[0])-4], "view")) {
        extern int fileModified;
        if (fileModified) {
            printf("can't read %s\n", argv[1]);
            return -1;
        }
        console_init();
        view_mode();
    } else {
        console_init();
        command_mode();
    }

    console_black();
    return 0;

}
