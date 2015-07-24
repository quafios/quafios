/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios C Standard Library.                         | |
 *        | |  -> API: Memory management.                          | |
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

#include <api/mman.h>
#include <api/fs.h>
#include <api/syscall.h>
#include <errno.h>

int brk(void *addr) {

    unsigned int new_brk = (unsigned int) addr;
    unsigned int cur_brk = syscall(SYS_BRK, new_brk);

    if (cur_brk != new_brk) {
        /* some error happened. */
        errno = ENOMEM;
        return -1;
    }

    return 0;

}

void *sbrk(int increment) {

    unsigned int cur_brk = syscall(SYS_BRK, 0);
    unsigned int new_brk = syscall(SYS_BRK, cur_brk + increment);

    if (new_brk != cur_brk + increment) {
        /* some error happened. */
        errno = ENOMEM;
        return NULL;
    }

    return (void *) cur_brk; /* the previous break; */

}

void *mmap(void *base, unsigned int size, unsigned int type,
           unsigned int flags, int fd, pos_t off) {

    mmap_arg_t args;
    int ret;

    args.base = base;
    args.size = size;
    args.type = type;
    args.flags = flags;
    args.fd = fd;
    args.off = off;

    ret = syscall(SYS_MMAP, &args);

    return (void *) ret;

}


int munmap(void *base, unsigned int size) {

    int ret;
    ret = syscall(SYS_MUNMAP, base, size);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;

}
