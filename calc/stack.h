/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Calculator.                                 | |
 *        | |  -> Stack header.                                    | |
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

#ifndef STACK_H
#define STACK_H

#include "boolean.h"

typedef struct node_s {
    struct node_s *next;
    void *val;
} node_t;

typedef struct {
    node_t *top;
    int    size;
} stack_t;

void stack_init(stack_t *);
bool stack_isEmpty(stack_t *);
void *stack_peek(stack_t *);
void *stack_pop(stack_t *);
void *stack_push(stack_t *, void *);
int  stack_size(stack_t *);
void stack_destroy(stack_t *);

#endif
