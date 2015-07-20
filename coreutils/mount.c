/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Core Utilities.                             | |
 *        | |  -> mount.                                           | |
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
#include <api/fs.h>

int main(int argc, char *argv[]) {

    char *fstype;
    char *devfile;
    char *mntpoint;

    if (argc == 3) {
        devfile  = "";
        mntpoint = argv[1];
        fstype   = argv[2];
    } else if (argc == 4) {
        devfile  = argv[1];
        mntpoint = argv[2];
        fstype   = argv[3];
    } else {
        fprintf(stderr,"USAGE: mount [DEVFILE] MOUNTPOINT FSTYPE\n");
        return -1;
    }

    if (mount(devfile, mntpoint, fstype, 0, NULL) < 0) {
        fprintf(stderr,"Mount operation failed.\n");
        return -1;
    }

    return 0;

}
