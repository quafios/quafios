/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Linked list header.                              | |
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

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <arch/type.h>

typedef struct linknode_str {
    struct linknode_str *next;
    uint32_t datum[10];
} __attribute__((packed)) linknode;

#define _linkedlist(TYPE)               \
            struct {                    \
                    TYPE *first;        \
                    TYPE *last;         \
                    unsigned int count; \
            }


#ifdef QUAFIOS_KERNEL
typedef  _linkedlist(linknode) __attribute__((packed)) linkedlist;
#endif

#endif
