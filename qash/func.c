/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Shell.                                      | |
 *        | |  -> Shell functions.                                 | |
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
#include <stdlib.h>
#include <sys/fs.h>

#include "func.h"
#include "interpret.h"

char curdir[FILENAME_MAX+1] = "curdir";

int shexit() {
    _exit(0);
    return 0;
}

int splash() {
    printf("Welcome to Quafios shell 2.0.1! ");
    printf("Type `help' if you are newbie.\n");
    printf("\n");
    return 0;
}

int help() {
    printf("Available Commands:\n");
    printf("- help                - version                        \n");
    printf("- cd DIRECTORY        - mknod FILE (r|d) [DEVNUM]      \n");
    printf("- pwd                 - link OLDFILE NEWFILE           \n");
    printf("- ls [DIRECTORY]      - unlink FILE                    \n");
    printf("- dir [DIRECTORY]     - rm FILE...                     \n");
    printf("- view FILE           - mkdir DIRECTORY                \n");
    printf("- edit FILE           - rmdir DIRECTORY                \n");
    printf("- cat FILE            - mount [DEVFILE] MNTPOINT FSTYPE\n");
    printf("- calc                - umount MNTPOINT                \n");
    printf("- echo                - exit                           \n");
    printf("- quit                 \n");
    printf("Report bugs to <iocoder@aol.com>\n");
    return 0;
}

int version() {
    printf("2.0.1\n");
    return 0;
}

int pwd() {
    getcwd(curdir, FILENAME_MAX+1);
    printf("%s\n", curdir);
    return 0;
}

int cd() {
    int err;
    if (argc != 2) {
        printf("Usage: cd TARGET\n");
        return -1;
    }
    if (err = chdir(argv[1]))
        printf("%s is not a directory.\n", argv[1]);
    return err;
}

int echo() {
    int i;
    for(i = 1; i < argc; i++) {
        if (i > 1)
            printf(" ");
        printf(argv[i]);
    }
    if (i > 1)
        printf("\n");
    return 0;
}
