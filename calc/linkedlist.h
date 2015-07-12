/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Calculator.                                 | |
 *        | |  -> Linked list header.                              | |
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

#ifndef _LINKEDLIST_H
#define _LINKEDLIST_H

#include "boolean.h"

typedef struct SNode_s {
    struct SNode_s *next; /* pointer to next node in the list. */
    void *val;            /* the value stored by this node.    */
} SNode;

typedef struct {
    SNode *head;
    int size;
} linkedlist;

/* Prototypes: */
void  linkedlist_init(linkedlist *);
void  linkedlist_add(linkedlist *, int, void *);
void  linkedlist_addLast(linkedlist *, void *);
void *linkedlist_get(linkedlist *, int);
void  linkedlist_set(linkedlist *, int, void *);
void  linkedlist_clear(linkedlist *);
bool  linkedlist_isEmpty(linkedlist *);
void  linkedlist_remove(linkedlist *, int);
int   linkedlist_size(linkedlist *);
linkedlist linkedlist_sublist(linkedlist *, int, int);
bool  linkedlist_contains(linkedlist *, void *);
void  linkedlist_print(linkedlist *);

#endif
