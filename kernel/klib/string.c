/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Strings.                                         | |
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

#include <arch/type.h>
#include <lib/string.h>

int strcmp(const char *str1, const char *str2) {
    uint32_t i = 0;
    while (str1[i] && str1[i] == str2[i]) i++;
    if (str1[i] > str2[i]) return i+1;
    else if (str1[i] < str2[i]) return -i-1;
    else return 0;
}

size_t strlen(const char * str) {
    uint32_t i = 0;
    while (str[i]) i++;
    return i;
}

void strcpy(char *dest, char *src) {
    while(*dest++ = *src++);
}

void strncpy(char *dest, char *src, int32_t count) {
    while(count-- && (*dest++ = *src++));
    if (count < 0 && *(dest-1))
        *dest = 0;
}

#if 0
int strSplit(char *pars, char splitter, int *count_ret, char ***vector_ret) {

    /* Calculate count of parameters: */
    int count = 1;
    for(int i = 0; pars[i]; i++)
        count += (pars[i] == splitter ? 1 : 0);

    /* allocate vector: */
    char **vector = kmalloc(count*sizeof(char *));
    if (vector == (char **) NULLPTR)
        return ENOMEM;

    /* create vector: */
    int i, j = 0, s;
    for (i = 0; i < count; i++) {
        int start = j;

        /* get length of current argument: */
        s = 0;
        for (j; pars[j] != splitter && pars[j]; j++)
            s++;

        /* allocate space for the argument: */
        vector[i] = (char *) kmalloc(s+1);
        if (vector[i] == (char *) NULLPTR) break;

        /* copy parameter. */
        for (j = start; j < start + s; j++)
            vector[i][j-start] = pars[j];
        vector[i][(j++)-start] = 0;
    }

    /* Handle Errors: */
    if (i != count) {
        for(i--; i >= 0; i--)
            kfree((void *) vector[i]);
        kfree((void *) vector);
        return ENOMEM;
    }

    /* return: */
    *count_ret = count;
    *vector_ret = vector;
    return 0;

}
#endif

void strcuttail(char *str, char chr) {
    int32_t i = strlen(str)-1;
    while (i >= 0 && str[i] == chr)
        str[i--] = 0;
}

String toString(char *str) {
    String ret;
    ret.size = strlen(str);
    ret.s = str;
    return ret;
}
