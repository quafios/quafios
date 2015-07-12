/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Calculator.                                 | |
 *        | |  -> unary operators resolver.                        | |
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
#include <string.h>
#include <stdio.h>

#include "prototype.h"
#include "linkedlist.h"
#include "token.h"
#include "boolean.h"

int solv(linkedlist *toklist, linkedlist *solved) {

    token_t *prev = NULL, *cur;
    int i = 0;

    linkedlist_init(solved);

    while(1) {
        if (i >= linkedlist_size(toklist))
            break;

        cur = linkedlist_get(toklist, i);

        if ((cur->op == '+' || cur->op == '-') &&
            (prev == NULL || (prev->type == TOKEN_OPERATOR &&
            prev->op != ')'))) {

            /* the + or - are unary if they are found
             * in the beginning of string, or after
             * another operator..
             */

            char op = cur->op;

            while (1) {

                if (++i == linkedlist_size(toklist)) {
                    /* end of expression, that's silly. */
                    return -1;
                }

                cur = linkedlist_get(toklist, i);

                if (cur->type == TOKEN_NUMBER) {
                    token_t *t = malloc(sizeof(token_t));
                    t->type = TOKEN_NUMBER;
                    t->op   = 0;
                    t->num  = do_op(0, cur->num, op);
                    linkedlist_addLast(solved, t);
                    break;
                } else if (cur->op == op) {
                    op = '+';
                } else if (cur->op != op &&
                      (cur->op=='+'||cur->op=='-')) {
                    op = '-';
                } else if (cur->op == '(') {
                    token_t *t = malloc(sizeof(token_t));
                    t->type = TOKEN_OPERATOR;
                    t->op   = op;
                    t->num  = 0;
                    linkedlist_addLast(solved, t);

                    t = malloc(sizeof(token_t));
                    t->type = TOKEN_OPERATOR;
                    t->op   = '(';
                    t->num  = 0;
                    linkedlist_addLast(solved, t);

                    break;
                } else {
                    /* the user entered +*) for ex.... */
                    return -1;
                }
            }

        } else {

            token_t *t = malloc(sizeof(token_t));
            t->type = cur->type;
            t->op   = cur->op;
            t->num  = cur->num;
            linkedlist_addLast(solved, t);

        }

        prev = cur;
        i++;

    }

    return 0;
}
