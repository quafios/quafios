/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Calculator.                                 | |
 *        | |  -> Linked lists.                                    | |
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
#include <stdio.h> /* for debugging issues */

#include "linkedlist.h"
#include "boolean.h"

/***************************************************************************/
/*                           linkedlist_init()                             */
/***************************************************************************/

void linkedlist_init(linkedlist *this) {
    this->head = NULL;
    this->size = 0;
}

/***************************************************************************/
/*                            linkedlist_add()                             */
/***************************************************************************/

void linkedlist_add(linkedlist *this, int index, void *element) {
    if (index < 0 || index > this->size) {
        /* printf("linkedlist_add(): Index out of bounds.\n"); */
    } else if (!index) {
        SNode *node = malloc(sizeof(SNode));
        node->next = this->head;
        node->val = element;
        this->head = node;
        this->size++;
    } else {
        SNode *node = malloc(sizeof(SNode));
        int i = 1;
        SNode *ptr = this->head;
        node->val = element;
        while (i++ < index)
            ptr = ptr->next;
        /* now ptr refers to the element after which
         * the "node" should be inserted.
         */
        node->next = ptr->next;
        ptr->next = node;
        this->size++;
    }
}

/***************************************************************************/
/*                         linkedlist_addLast()                            */
/***************************************************************************/

void linkedlist_addLast(linkedlist *this, void *element) {
    linkedlist_add(this, this->size, element);
}

/***************************************************************************/
/*                           linkedlist_get()                              */
/***************************************************************************/

void *linkedlist_get(linkedlist *this, int index) {
    if (index < 0 || index >= this->size) {
        /* warning("linkedlist_get(): Index out of bounds.\n"); */
        return NULL;
    } else {
        SNode *ptr = this->head;
        int i = 0;
        while (i++ < index)
            ptr = ptr->next;
        return ptr->val;
    }
}

/***************************************************************************/
/*                            linkedlist_set()                             */
/***************************************************************************/

void linkedlist_set(linkedlist *this, int index, void *element) {
    if (index < 0 || index >= this->size) {
        /*warning("linkedlist_set(): Index out of bounds.\n"); */
    } else {
        SNode *ptr = this->head;
        int i = 0;
        while (i++ < index)
            ptr = ptr->next;
        ptr->val = element;
    }
}

/***************************************************************************/
/*                           linkedlist_clear()                            */
/***************************************************************************/

void linkedlist_clear(linkedlist *this) {
    while(this->size)
        linkedlist_remove(this, 0);
}

/***************************************************************************/
/*                         linkedlist_isEmpty()                            */
/***************************************************************************/

bool linkedlist_isEmpty(linkedlist *this) {
    return this->size ? true : false;
}

/***************************************************************************/
/*                          linkedlist_remove()                            */
/***************************************************************************/

void linkedlist_remove(linkedlist *this, int index) {
    if (index < 0 || index >= this->size) {
        /*warning("linkedlist_remove(): Index out of bounds.\n"); */
    } else if (!index) {
        SNode *node = this->head;
        this->head = node->next;
        free(node);
        this->size--;
    } else {
        int i = 1;
        SNode *ptr = this->head;
        SNode *node;
        while (i++ < index)
            ptr = ptr->next;
        /* now ptr refers to the element after which
         * the "node" to be removed exists.
         */
        node = ptr->next;
        ptr->next = node->next;
        free(node);
        this->size--;
    }
}

/***************************************************************************/
/*                           linkedlist_size()                             */
/***************************************************************************/

int linkedlist_size(linkedlist *this) {
    return this->size;
}

/***************************************************************************/
/*                         linkedlist_sublist()                            */
/***************************************************************************/

linkedlist linkedlist_sublist(linkedlist *this, int fromIndex, int toIndex) {

    linkedlist ret = {NULL, 0};
    int i = 0;
    SNode *ptr;

    /* check borders: */
    if (fromIndex < 0 || fromIndex >= this->size) {
        /* warning("linkedlist_sublist(): fromIndex out of bounds.\n"); */
        return ret;
    }

    if (toIndex < 0 || toIndex >= this->size) {
        /* warning("linkedlist_sublist(): toIndex out of bounds.\n"); */
        return ret;
    }

    if (toIndex < fromIndex) {
        /* warning("linkedlist_sublist(): indices are invalid.\n"); */
        return ret;
    }

    linkedlist_init(&ret); /* DS = ES = SS :D */
    i = 0;
    ptr = this->head;
    while (i <= toIndex) {
        if (i >= fromIndex)
            linkedlist_addLast(&ret, ptr->val);
        ptr = ptr->next;
        i++;
    }
    return ret;
}

/***************************************************************************/
/*                         linkedlist_contains()                           */
/***************************************************************************/

bool linkedlist_contains(linkedlist *this, void *o) {
    SNode *ptr = this->head;
    while (ptr) {
        if (ptr->val == o)
            return true;
        ptr = ptr->next;
    }
    return false;
}

/***************************************************************************/
/*                          linkedlist_print()                             */
/***************************************************************************/

void linkedlist_print(linkedlist *this) {
    SNode *ptr = this->head;
    printf("SLL(%d): ", this->size);
    if (ptr == NULL)
        printf("(null)");
    while (ptr) {
        printf("%s%p", ptr == this->head ? "":" -> ", ptr->val);
        ptr = ptr->next;
    }
    printf("\n");
}
