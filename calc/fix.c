/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Calculator.                                 | |
 *        | |  -> Infix-to-postfix conversion.                     | |
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

#include <stdio.h>
#include <stdlib.h>

#include "prototype.h"
#include "linkedlist.h"
#include "stack.h"
#include "token.h"
#include "boolean.h"

/* part 2 of syntax analysis.
 * a simple algorithm that converts infix expressions into postfix...
 * it reports errors of syntax...
 */

int getPrec(char op) {
    /* get precedence. */
    switch(op) {
        case '+':
        case '-':
        return 1;

        case '*':
        case '/':
        return 2;


        case '(':
        case ')':
        return 3;

        default:
        return 0;
    }
}

int inToPostFix(linkedlist *toklist, linkedlist *post) {

    stack_t stk; /* operator stack */
    int i;
    int err = 0;

    linkedlist_init(post);
    stack_init(&stk);

    for (i = 0; i < linkedlist_size(toklist); i++) {
        token_t *t = linkedlist_get(toklist, i);
        token_t *x;
        if (t->type == TOKEN_NUMBER) {
            /* ordinary number... add it to the postfix string: */
            linkedlist_addLast(post, t);
            continue;
        }

        /* OMG operator :o
         * it's kinda tricky ;)
         * don't worry we can do it :D
         */

        /* parenthesis:  */
        /* ------------- */
        if (t->op == '(') {
            /* force it to be pushed: */
            stack_push(&stk, t);
            continue;
        }

        if (t->op == ')') {
            /* pop all operators, till we reach the opening parenthesis */
            int count = 0;
            while(1) {
                if (stack_isEmpty(&stk)) {
                    return -1; /* syntax error */
                }
                x = stack_pop(&stk);
                if (x->op == '(') {
                    if (count == 0) {
                        return -1;
                    }
                    /* done. */
                    break;
                }
                count++;
                linkedlist_addLast(post, x);
            }
            continue;
        }

        /* binary operators:  */
        /* ------------------ */
        while (1) {
            if (stack_isEmpty(&stk)) {
                /* push the operator. */
                stack_push(&stk, t);
                break;
            }
            /* check the precedence of last pushed operator: */
            x = stack_peek(&stk);
            if (x->op=='(' || getPrec(t->op)>getPrec(x->op)){
                stack_push(&stk, t);
                break;
            }

            linkedlist_addLast(post, stack_pop(&stk));
        }

    }

    /* pop out all remaining operators: */
    while (!stack_isEmpty(&stk)) {
        token_t *x = stack_pop(&stk);
        if (x->op == '(') err = -1;
        linkedlist_addLast(post, x);
    }
    return err;

}
