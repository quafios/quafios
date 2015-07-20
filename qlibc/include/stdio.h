/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios C Standard Library.                         | |
 *        | |  -> Standard I/O header.                             | |
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

#ifndef STDIO_H
#define STDIO_H

/* Includes:  */
/* ========== */
#include <sys/fs.h>    /* includes FILENAME_MAX.           */
#include <sys/error.h>
#include <sys/mm.h>    /* includes NULL definition.        */
#include <arch/type.h> /* includes size_t type definition. */
#include <stdarg.h>    /* for va_list;                     */

/* Macros:  */
/* ======== */
#define BUFSIZ          4096
#define EOF             (-1)
/* FILENAME_MAX already defined */
#define FOPEN_MAX       FD_MAX
#define L_tmpnam        1
/* NULL already defined. */
#define TMP_MAX         FOPEN_MAX

/* Types:  */
/* ======= */
typedef struct {

    /* FILE ACCESS: */
    #define F_READ      0x01
    #define F_WRITE     0x02
    #define F_APPEND    0x04
    #define F_CREAT     0x08
    int access;

    /* TEXT/BINARY: */
    #define F_TEXT      0x00
    #define F_BINARY    0x01
    int textbin;

    /* BUFFERING: */
    #define _IOFBF      0x00 /* Full buffering */
    #define _IOLBF      0x01 /* Line buffering */
    #define _IONBF      0x02 /*  No  buffering */
    int   bufmod;
    #define _IOIBF      0x00 /* internal buffer used         */
    #define _IOEBF      0x01 /* external buffer used         */
    #define _IODBF      0x02 /* dynamically allocated buffer */
    char  bufsrc;
    int   bufsiz;
    char *buffer;
    char  getChar;           /* used by ungetc */
    int   getCharIsValid;

    /* ORIENTATION: */

    /* INDICATORS: */
    int   err;
    int   eof;
    pos_t pos;

} FILE;
typedef pos_t fpos_t;
/* size_t is already defined. */

/* Streams:  */
/* ========= */
extern FILE __streams[FD_MAX];
extern char *__defbuf[FD_MAX];
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

/* Functions:  */
/* =========== */
/* streams: */
void __streams_init();

/* Operations on files: */

/* File access: */
void __mode_to_flags(const char *mode, int *access, int *textbin);
void __resetbuf(FILE *f, char isInitialized);
FILE *fdopen(int fd, const char *mode);
FILE *fopen(const char *filename, const char *mode);
int fclose(FILE *f);

/* Formatted input/output: */
int fprintf(FILE *stream, const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);
int printf(const char *format, ...);
int scanf(const char *format, ...);
int sprintf(const char *str, const char *format, ...);
int sscanf(const char *str, const char *format, ...);
int vfprintf(FILE *stream, const char *format, va_list arg);
int vprintf(const char *format, va_list arg);
int vsprintf(const char *str, const char *format, va_list arg);

/* Character input/output: */
int fgetc(FILE *stream);
char *fgets(char *str, int num, FILE *stream);
int fputc(int character, FILE *stream);
int fputs(const char *str, FILE *stream);
int getc(FILE *stream);
int getchar(void);
char *gets(char *str);
int putc(int character, FILE *stream);
int putchar(int character);
int puts(const char *str);
int ungetc(int character, FILE *stream);

/* Direct input/output: */
size_t __frw(void *ptr, size_t size, size_t count, FILE *stream, int dir);
size_t fread(void *ptr, size_t size, size_t count, FILE *stream);
size_t fwrite(void *ptr, size_t size, size_t count, FILE *stream);

/* File positioning: */
void __chpos(FILE *stream, pos_t pos);

/* Error-handling: */
void clearerr(FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);
void perror(const char *str);

#endif
