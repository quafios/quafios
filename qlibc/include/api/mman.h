/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios C Standard Library.                         | |
 *        | |  -> API: Memory management header.                   | |
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

#ifndef __API_MMAN_H
#define __API_MMAN_H

#include <sys/mm.h>    /* kernel memory structures. */
#include <sys/error.h> /* kernel error codes.       */
#include <sys/fs.h>    /* for pos_t type            */

int brk(void *addr);
void *sbrk(int increment);
void *mmap(void *base, unsigned int size, unsigned int type,
           unsigned int flags, int fd, pos_t off);

#endif
