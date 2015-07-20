/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios C Standard Library.                         | |
 *        | |  -> Standard I/O: Character I/O.                     | |
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

/****************************************************************************/
/*                                 fgetc()                                  */
/****************************************************************************/

int fgetc(FILE *stream) {
    int ret = 0;
    if (fread(&ret, sizeof(char), 1, stream) != 1)
        return EOF; /*error... */
    return ret;
}

/****************************************************************************/
/*                                 fgets()                                  */
/****************************************************************************/

char *fgets(char *str, int num, FILE *stream) {
    int i;
    for(i = 0; i < num-1; i++) {
        int c = fgetc(stream);
        if (c == EOF) {
            if (i > 0) str[i] = '\0';
            return NULL; /* error */
        }
        str[i] = c;
        if (c == '\n') {
            i++;
            break;
        }
    }
    str[i] = '\0';
    return str;
}

/****************************************************************************/
/*                                 fputc()                                  */
/****************************************************************************/

int fputc(int character, FILE *stream) {
    if (fwrite(&character, sizeof(char), 1, stream) != 1)
        return EOF;
    return character;
}

/****************************************************************************/
/*                                 fputs()                                  */
/****************************************************************************/

int fputs(const char *str, FILE *stream) {
    while(*str) {
        if (fputc(*str, stream) == EOF)
            return EOF;
        str++;
    }
    return 0; /* success. */
}

/****************************************************************************/
/*                                 getc()                                   */
/****************************************************************************/

int getc(FILE *stream) {
    return fgetc(stream);
}

/****************************************************************************/
/*                                getchar()                                 */
/****************************************************************************/

int getchar(void) {
    return getc(stdin);
}

/****************************************************************************/
/*                                 gets()                                   */
/****************************************************************************/

char *gets(char *str) {
    int i = 0;
    while(1) {
        int c = fgetc(stdin);
        if (c == EOF) {
            if (i > 0) str[i] = '\0';
            return NULL; /* error */
        }
        if (c == '\n')
            break;
        str[i++] = c;
    }
    str[i] = '\0';
    return str;
}

/****************************************************************************/
/*                                  putc()                                  */
/****************************************************************************/

int putc(int character, FILE *stream) {
    return fputc(character, stream);
}

/****************************************************************************/
/*                                putchar()                                 */
/****************************************************************************/

int putchar(int character) {
    return putc(character, stdout);
}

/****************************************************************************/
/*                                  puts()                                  */
/****************************************************************************/

int puts(const char *str) {
    if (fputs(str, stdout) == EOF)
        return EOF;
    if (putchar('\n') == EOF)
        return EOF;
    return 0;
}

/****************************************************************************/
/*                                 ungetc()                                 */
/****************************************************************************/

int ungetc(int character, FILE *stream) {
    if (character != EOF) {
        stream->getChar = character;
        stream->getCharIsValid = 1;
        stream->pos--;
    }
    return character;
}
