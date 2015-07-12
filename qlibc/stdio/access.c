/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios C Standard Library.                         | |
 *        | |  -> Standard I/O: File access procedures.            | |
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
#include <errno.h>
#include <api/fs.h>

/****************************************************************************/
/*                            mode_to_flags()                               */
/****************************************************************************/

void __mode_to_flags(const char *mode, int *access, int *textbin) {
    int i = 0;

    *access  = 0;
    *textbin = F_TEXT;

    while (mode[i]) {
        switch(mode[i++]) {
            case 'r':
            *access |= F_READ;
            break;

            case 'w':
            *access |= F_WRITE;
            *access |= F_CREAT;
            break;

            case 'a':
            *access |= F_WRITE;
            *access |= F_APPEND;
            break;

            case '+':
            *access |= F_READ;
            *access |= F_WRITE;
            break;

            case 'b':
            *textbin = F_BINARY;
            break;

            default:
            /* just ignore -_- */
            break;
        }
    }
}

/****************************************************************************/
/*                               resetbuf()                                 */
/****************************************************************************/

void __resetbuf(FILE *f, char isInitialized) {
    /* the file descriptor */
    int fd;

    if (!isInitialized) {
        f->getCharIsValid = 0;
    }

    /* interpret the fd of the stream. */
    fd = f-__streams;

    /* reset the buffer of the stream to default buffer.
     * this also resets buffering mode to "not buffered".
     * a default buffer for every stream
     * is always available, even it is not buffered.
     */
    if (isInitialized && f->bufsrc == _IODBF) {
        /* dynamically allocated buffer should be first deallocated. */
        /* TODO: free(f->buffer); */
    }

    /* reset buffer: */
    f->bufsrc = _IOIBF; /* internal buffer */
    f->bufsiz = BUFSIZ; /* default size    */
    f->buffer = __defbuf[fd];

    /* reset buffering mode */
    f->bufmod = _IONBF; /* not buffered. */
}

/****************************************************************************/
/*                                fdopen()                                  */
/****************************************************************************/

FILE *fdopen(int fd, const char *mode) {
    FILE *f;
    pos_t pos = 0;

    /* make sure "fd" is valid by seeking... */
    pos = seek(fd, pos, SEEK_CUR);
    if (pos < 0)
        return NULL;

    /* fd is valid... */
    f = &__streams[fd];

    /* initialize f structure: */
    __mode_to_flags(mode, &(f->access), &(f->textbin));
    clearerr(f);
    __chpos(f, pos);
    __resetbuf(f, 0 /* not initialized */);

    /* return: */
    return f;
}

/****************************************************************************/
/*                                 fopen()                                  */
/****************************************************************************/

FILE *fopen(const char *filename, const char *mode) {
    int fd, err;
    int access, textbin;

    /* check if mode contains 'w':  */
    /* ---------------------------- */
    __mode_to_flags(mode, &access, &textbin);
    if (access & F_CREAT) {

        /* create a new empty file with the same name. */
        mknod((char *) filename, FT_REGULAR, 0);

        /* truncate: */
        if (truncate((char *) filename, 0) < 0) {
            return NULL;
        }

    }

    /* call open() system call to get the fd:  */
    /* --------------------------------------- */
    fd = open((char *) filename, NULL);
    if (fd < 0)
        return NULL;

    /* Create FILE structure by means of fd:  */
    /* -------------------------------------- */
    return fdopen(fd, mode);
}

/****************************************************************************/
/*                                fclose()                                  */
/****************************************************************************/

int fclose(FILE *f) {
    int fd = f-__streams;
    if (close(fd) < 0)
        return EOF;
    return ESUCCESS;
}
