/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Calculator.                                 | |
 *        | |  -> Postfix expression evaluation.                   | |
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

double do_op(double n1, double n2, char op) {
    switch(op) {
        case '+':
        return n1+n2;

        case '-':
        return n1-n2;

        case '*':
        return n1*n2;

        case '/':
        return n1/n2;

        default:
        return 0;
    }
}

int eval(linkedlist *expr, double *val) {

    int i;
    stack_t stk; /* operator stack */

    stack_init(&stk);

    for(i = 0; i < linkedlist_size(expr); i++) {
        token_t *t = linkedlist_get(expr, i);
        token_t *n2, *n1, *res;

        /* if number, push to stack: */
        if (t->type == TOKEN_NUMBER) {
            stack_push(&stk, t);
            continue;
        }

        /* all operators in the postfix expression are binary. */
        if (stack_size(&stk) < 2) {
            while(!stack_isEmpty(&stk))
                free(stack_pop(&stk));
            /* free the remaining tokens in expr: */
            while(i < linkedlist_size(expr))
                free(linkedlist_get(expr, i++));
            return -1; /* error. */
        }
        n2 = stack_pop(&stk);
        n1 = stack_pop(&stk);

        res = malloc(sizeof(token_t));
        res->type = TOKEN_NUMBER;
        res->op   = 0;
        res->num  = do_op(n1->num, n2->num, t->op);
        stack_push(&stk, res);

        free(n2);
        free(n1);
        free(t);
    }

    if (stack_size(&stk) != 1) {
        while(!stack_isEmpty(&stk)) {
            free(stack_pop(&stk));
        }
        return -1; /* error. */
    }

    *val = ((token_t *) stack_peek(&stk))->num;
    free(stack_pop(&stk));
    return 0;

}
