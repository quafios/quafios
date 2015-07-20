/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios C Standard Library.                         | |
 *        | |  -> Standard I/O: Direct I/O.                        | |
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
#include <errno.h>
#include <sys/fs.h>

/****************************************************************************/
/*                                  frw()                                   */
/****************************************************************************/

size_t __frw(void *ptr, size_t size, size_t count, FILE *stream, int dir) {
    int fd = stream - __streams;
    int i = 0; /* number of successfuly read blocks. */

    if (stream->bufmod == _IONBF) {
        /* block by block... */
        while(count--) {

            /* do the system call; */
            int ret = 0;
            int asize = 0;
            int err = 0;
            if (dir == 0) {
                if (stream->getCharIsValid) {
                    char *cptr = ptr;
                    stream->getCharIsValid = 0;
                    cptr[0] = stream->getChar;
                    asize = 1;
                    ret = read(fd, &cptr[1],size-1);
                } else {
                    ret = read(fd, ptr, size);
                }
            } else
                ret = write(fd, ptr, size);

            /* translate value of "ret": */
            if (ret < 0)
                err = errno;
            else
                asize += ret;

            /* update pointers */
            ptr = ((char *) ptr) + asize;
            stream->pos += asize;

            /* check if there is an error. */
            if (err) {
                stream->err = ret;
                break;
            } else if (asize < size) {
                stream->eof = 1;
                break;
            }

            /* continue: */
            i++;
            }
    }
    return i;
}

/****************************************************************************/
/*                                 fread()                                  */
/****************************************************************************/

size_t fread(void *ptr, size_t size, size_t count, FILE *stream) {
    return __frw(ptr, size, count, stream, 0 /* read */);
}

/****************************************************************************/
/*                                fwrite()                                  */
/****************************************************************************/

size_t fwrite(void *ptr, size_t size, size_t count, FILE *stream) {
    return __frw(ptr, size, count, stream, 1 /* write */);
}
