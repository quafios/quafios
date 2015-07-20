/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Hardware Resources.                              | |
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
#include <i386/asm.h>
#include <sys/resource.h>

void iowrite(uint32_t size, uint32_t type, uint32_t val,
             uint32_t base, uint32_t offset) {
    /* TODO: should use physical memory instead of virtual one. */
    /* TODO: Move arch-dependant code to arch */
    if (size == 1 && type == RESOURCE_TYPE_MEM) {
        *((uint8_t  *) (base + offset)) = (uint8_t ) val;
    } else if (size == 2 && type == RESOURCE_TYPE_MEM) {
        *((uint16_t *) (base + offset)) = (uint16_t) val;
    } else if (size == 4 && type == RESOURCE_TYPE_MEM) {
        *((uint32_t *) (base + offset)) = (uint32_t) val;
    } else if (size == 1 && type == RESOURCE_TYPE_PORT) {
        outb((uint8_t ) val, base + offset);
    } else if (size == 2 && type == RESOURCE_TYPE_PORT) {
        outw((uint16_t) val, base + offset);
    } else if (size == 4 && type == RESOURCE_TYPE_PORT) {
        outl((uint32_t) val, base + offset);
    }
}

uint32_t ioread(uint32_t size, uint32_t type, uint32_t base, uint32_t offset) {
    /* TODO: should use physical memory instead of virtual one. */
    /* TODO: Move arch-dependant code to arch */
    if (size == 1 && type == RESOURCE_TYPE_MEM) {
        return *((uint8_t  *) (base + offset));
    } else if (size == 2 && type == RESOURCE_TYPE_MEM) {
        return *((uint32_t *) (base + offset));
    } else if (size == 4 && type == RESOURCE_TYPE_MEM) {
        return *((uint32_t *) (base + offset));
    } else if (size == 1 && type == RESOURCE_TYPE_PORT) {
        return inb(base + offset);
    } else if (size == 2 && type == RESOURCE_TYPE_PORT) {
        return inw(base + offset);
    } else if (size == 4 && type == RESOURCE_TYPE_PORT) {
        return inl(base + offset);
    }
}
