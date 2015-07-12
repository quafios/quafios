/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> Linked Lists.                                    | |
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

#include <arch/type.h>
#include <sys/mm.h>
#include <lib/linkedlist.h>

void linkedlist_add(linkedlist *ll, linknode *node) {
    /* Adds an element in an order of O(1) :D */
    node->next = ll->first;
    ll->first = node;
    if (!(ll->count)) ll->last = node;
    ll->count++;
}

void linkedlist_addafter(linkedlist *ll, linknode *node, linknode *prev) {
    /* Adds an element after another specified one. */
    if (ll->count && prev != NULL) {
        ll->count++;
        node->next = prev->next;
        prev->next = node;
        if (prev == ll->last)
            ll->last = node;
    } else {
        linkedlist_add(ll, node);
    }
}

void linkedlist_addlast(linkedlist *ll, linknode *node) {
    /* Adds an element in the end of the array. */
    node->next = NULL;
    if (ll->count) {
        ll->last->next = node;
    } else {
        ll->first = node;
    }
    ll->last = node;
    ll->count++;
}

void linkedlist_remove(linkedlist *ll, linknode *node, linknode *prev) {
    /* Removes an element in order of O(1) :D */
    if (prev == NULL)
        /* Removing first element: */
        ll->first = node->next;
    else
        prev->next = node->next;
    if ((uint32_t) ll->last == (uint32_t) node) ll->last = prev;
    ll->count--;
}

void  linkedlist_aremove(linkedlist *ll, linknode *node) {
    /* Removes an element in order of O(N) :( */
    linknode *prev = NULL;
    linknode *i;
    for(i = ll->first; i != NULL; i = i->next) {
        if (i == node) {
            linkedlist_remove(ll, node, prev);
            break;
        } else {
            prev = i;
        }
    }
}

void linkedlist_init(linkedlist *ll) {
    ll->first = NULL;
    ll->last  = NULL;
    ll->count = 0;
}
