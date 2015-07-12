/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> Hardware resources.                              | |
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

/* an interface to resources used by devices. */

#ifndef RESOURCE_H
#define RESOURCE_H

#include <arch/type.h>

#define RESOURCE_TYPE_MEM       0
#define RESOURCE_TYPE_PORT      1
#define RESOURCE_TYPE_IRQ       2
#define RESOURCE_TYPE_DMA       3

typedef struct {
    uint32_t type;
    union {
        struct {
            uint32_t base;
            uint32_t size;
        } mem;
        struct {
            uint32_t base;
            uint32_t size;
        } port;
        struct {
            uint32_t number;
            uint32_t reserved;
        } irq;
        struct {
            uint32_t channel;
            uint32_t devid;
        } dma;
    } data;
} resource_t;

/* Resource list */
typedef struct {
    uint32_t    count; /* count of resources. */
    resource_t* list;  /* the actual vector.  */
} reslist_t;

#endif
