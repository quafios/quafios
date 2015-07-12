/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios C Standard Library.                         | |
 *        | |  -> stdlib: string conversion.                       | |
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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int atoi(const char *str) {
    unsigned int i = 0;
    unsigned int num = 0;
    signed int sign = 1;

    /* skip white spaces: */
    while (isspace(str[i]))
        i++;

    /* sign? */
    if (str[i] == '-') {
        sign = -1;
        i++;
    } else if (str[0] == '+') {
        sign = 1;
        i++;
    }

    while (str[i] >= '0' && str[i] <= '9')
        num = num*10 + (str[i++] - '0');

    return num*sign;
}

double strtod(const char *str, char **endptr) {

    double ret = 0;
    int i = 0;
    int negative = 0;
    int digits = 0;

    /* skip white spaces: */
    while (isspace(str[i]))
        i++;

    /* sign character: */
    if (str[i] == '+') {
        negative = 0;
        i++;
    } else if (str[i] == '-') {
        negative = 1;
        i++;
    }

    /* before float point: */
    while(str[i] >= '0' && str[i] <= '9') {
        ret = ret*10 + (str[i]-'0');
        digits++;
        i++;
    }

    /* float point? */
    if (str[i] == '.') {
        double tmp = 1;
        i++;
        while(str[i] >= '0' && str[i] <= '9') {
            tmp *= 10;
            ret = ret + (str[i]-'0')/tmp;
            digits++;
            i++;
        }
    }

    /* valid till now? */
    if (!digits) {
        /* no digits have been read! */
        return 0;
    }

    /* evaluate sign: */
    if (negative)
        ret = -ret;

    /* TODO: exponent? */

    if (endptr) {
        *endptr = (char *) &str[i];
    }

    return ret;

}

char *itoa(int value, char *str, int base) {

    int i = 0, j = 0;

    char tmp[20];

    if (base > 16)
        return NULL;

    if (!value)
        return strcpy(str, "0");

    if (value < 0) {
        value = -value;
        str[j++] = '-';
    }

    while (value) {
        tmp[i++] = value%base;
        value /= base;
    }

    while (i > 0)
        str[j++] = "0123456789abcdef"[tmp[--i]];

    str[j] = 0;

    return str;

}
