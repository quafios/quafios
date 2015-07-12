/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios C Standard Library.                         | |
 *        | |  -> String operations.                               | |
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

#include <string.h>
#include <stdio.h>

static char *nextToken;

int strcmp(const char *str1, const char *str2) {
    unsigned int i = 0;
    while (str1[i] == str2[i] && str1[i]) i++;
    if (str1[i] > str2[i]) return 1;
    else if (str1[i] < str2[i]) return -1;
    else return 0;
}

size_t strlen(const char *str) {
    size_t i = 0;
    while (str[i]) i++;
    return i;
}

char *strcpy(char *destination, const char *source) {
    char *ret = destination;
    while(*destination++ = *source++);
    return ret;
}

char *strcat(char *destination, const char *source) {
    return strcpy(&destination[strlen(destination)], source);
}

char *strtok(char *str, const char *delimiters) {

    char *curToken;

    /* first call? */
    if (str != (char *) NULL)
        nextToken = str;

    curToken = nextToken;

    /* step 1:
     * if curToken starts with one of the
     * delimiters, skip them all..
     */
    while(*curToken) {
        const char *d = delimiters;
        while(*d) {
            if (*curToken == *d)
                break;
            d++; /* increase if non-equal. */
        }

        if (*d)
            curToken++; /* *curToken matches a delimiter. */
        else
            break;
    }
    nextToken = curToken; /* temporarily. */

    /* step 2:
     * we reached the end of str?
     */
    if (*curToken == 0)
        return NULL; /* done. */

    /* step 3:
     * calculate the new nextToken value.
     * move forward until we reach a delimeter or end of string.
     */
    while(*nextToken) {
        const char *d = delimiters;
        while(*d) {
            if (*nextToken == *d)
                break;
            d++; /* increase if non-equal. */
        }

        if (*d) /* *nextToken matches a delimiter. */
            break;
        else
            nextToken++; /* increase if not matching. */
    }
    if(*nextToken) {
        *nextToken = 0;
        nextToken++;
    }

    /* done. */
    return curToken;
}
