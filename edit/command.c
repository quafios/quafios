/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Text Editor.                                | |
 *        | |  -> Command mode operations.                         | |
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

char *status_str = "Press `Enter' to go into edit mode or `Q' to quit";
char *failure    = "Can't save the file! Press any key to continue";

void command_mode() {
    while(1) {
        char c;
        console_title("Command mode");
        console_status(status_str);
        c = getchar();
        if (c == '\n')
            edit_mode();
        else if (c == 'q' || c == 'Q') {
            extern int fileModified;
            if (!fileModified)
                return;
            console_status("Save changes? (y/n/c)");
            while(1) {
                c = getchar();
                if (c == 'y' || c == 'Y') {
                    if (file_save()) {
                        console_status(failure);
                        getchar();
                        break;
                    }
                    return; /* done. */
                } else if (c == 'n' || c == 'N') {
                    return; /* just return; */
                } else if (c == 'c' || c == 'C') {
                    break;  /* cancel; */
                }
            }
        }
    }
}

