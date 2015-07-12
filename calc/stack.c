/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Calculator.                                 | |
 *        | |  -> Stacks.                                          | |
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

#include <stdlib.h>
#include "stack.h"

/* ========================================================================= */
/*                                  init()                                   */
/* ========================================================================= */

void stack_init(stack_t *this) {
    this->top = NULL;
    this->size = 0;
}

/* ========================================================================= */
/*                                 isEmpty()                                 */
/* ========================================================================= */

bool stack_isEmpty(stack_t *this) {
    return this->size == 0 ? true : false;
}

/* ========================================================================= */
/*                                  peek()                                   */
/* ========================================================================= */

void *stack_peek(stack_t *this) {
    if (stack_isEmpty(this))
        return NULL; /* underflow. */
    return this->top->val;
}

/* ========================================================================= */
/*                                   pop()                                   */
/* ========================================================================= */

void *stack_pop(stack_t *this) {
    void *val;
    node_t *next;
    if (stack_isEmpty(this))
        return NULL; /* underflow. */
    val = this->top->val;
    next = this->top->next;
    free(this->top);
    this->top = next;
    this->size--;
    return val;
}

/* ========================================================================= */
/*                                  push()                                   */
/* ========================================================================= */

void *stack_push(stack_t *this, void *val) {
    node_t *node;
    if (val == NULL)
        return NULL; /* not acceptable. */
    node = malloc(sizeof(node_t));
    if (node == NULL) {
        return NULL; /* virtual memory error. */
    }
    node->val = val;
    node->next = this->top;
    this->top = node;
    this->size++;
}

/* ========================================================================= */
/*                                  size()                                   */
/* ========================================================================= */

int stack_size(stack_t *this) {
    return this->size;
}

/* ========================================================================= */
/*                                destroy()                                  */
/* ========================================================================= */

void stack_destroy(stack_t *this) {
    while(stack_pop(this));
}
