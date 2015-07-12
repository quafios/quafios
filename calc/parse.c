/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Calculator.                                 | |
 *        | |  -> parse() procedure.                               | |
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

/* lexical and syntax analysis, part 1. */

int parse(char *expr, linkedlist *ret) {

    char *tmp, *tok;

    /* list of tokens. */
    linkedlist toklist;
    linkedlist_init(&toklist);

    /* parse the expression (expr): */
    tmp = malloc(strlen(expr)+1);
    strcpy(tmp, expr);

    /* split the string by spaces: */
    tok = strtok(tmp, " "); /* delete spaces. */
    do {
        int i = 0;
        while(tok[i]) {
            char type;
            char op;
            double num;
            char *ep;
            token_t *t;
            switch(tok[i]) {
                case '+':
                case '-':
                case '*':
                case '/':
                case '(':
                case ')':
                /* operator token :D */
                type = TOKEN_OPERATOR;
                op   = tok[i++];
                num  = 0;
                break;

                default:
                /* not an operator, should be a regular real number: */
                type = TOKEN_NUMBER;
                op = 0;
                ep = &tok[i];
                num  = strtod(&tok[i], &ep);
                if (ep == &tok[i]) {
                    /* unknown character! */
                    return -1;
                }
                i = ((int)(ep-tok))*sizeof(char);
            }
            t = malloc(sizeof(token_t));
            t->type = type;
            t->op   = op;
            t->num  = num;
            linkedlist_addLast(&toklist, t);
        }
    } while(tok = strtok(NULL, " "));

    free(tmp);
    *ret = toklist;
    return 0;
}
