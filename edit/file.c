/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Text Editor.                                | |
 *        | |  -> File operations.                                 | |
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
#include <api/mman.h>

char *filename;
char *fileMem;
int fileSize = 0;
int offset = 0;
int fileModified = 0;

int file_load(char *fname) {
    FILE *fp = fopen(fname, "r");
    filename = fname;
    fileMem = sbrk(0);
    if (fp) {
        /* read file */
        int c;
        while((c = getc(fp)) != EOF) {
            sbrk(1);
            fileMem[fileSize++] = (char) c;
        }
        fclose(fp);
    } else {
        /* invalid name */
        fileModified = 1;
    }
    return 0;
}

void file_insert(char c, int off) {
    int i;
    fileModified = 1;
    sbrk(1);
    fileSize++;
    for (i = fileSize-1; i > off; i--)
        fileMem[i] = fileMem[i-1];
    fileMem[off] = c;
}

void file_del(int off) {
    int i;
    fileModified = 1;
    for (i = off; i < fileSize-1; i++)
        fileMem[i] = fileMem[i+1];
    fileSize--;
    sbrk(-1);
}

int file_save() {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
        return -1; /* failed.. */
    if (fileSize && (!fwrite(fileMem, fileSize, 1, fp))) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}
