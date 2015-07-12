/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Calculator.                                 | |
 *        | |  -> main() procedure.                                | |
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
#include <string.h>

#include "prototype.h"
#include "linkedlist.h"
#include "token.h"

#define BUFFER_SIZE   4096
char buffer[BUFFER_SIZE];

/* comment the following line if not debug mode. */
/* #define SHOW_POSTFIX */

int main() {

    printf("Quafios Calculator 1.0.2\n");
    printf("This software is free open source. For license details\n");
    printf("type: \"license\". Type \"quit\" to exit or");
    printf(" \"help\" for help.\n\n");

    /* The infinite loop: */
    while(1) {

        int i = 0;
        linkedlist expr, infix, postfix;
        double val;

        printf("; ");

        while(1) {
            char x = getchar();
            if (x == '\n') {
                buffer[i] = 0;
                break;
            }
            if (i < BUFFER_SIZE-1)
                buffer[i++] = x;
        }

        if (!strcmp(buffer, "quit")) {
            break;
        } else if (!strcmp(buffer, "license")) {
            printf("This program is licensed under GNU ");
            printf("General Public License v3. Please\n");
            printf("visit Quafios website (http://www.");
            printf("quafios.com) for more details.\n");
            continue;
        } else if (!strcmp(buffer, "help")) {
            printf("Just type some equation then press ");
            printf("enter. To exit, type ");
            printf("\"quit\" instead.\n");
            continue;
        } else if (!strcmp(buffer, "")) {
            continue;
        }

        if (parse(buffer, &expr)) {
            printf("Synatx Error (#C0)!\n");
            continue;
        };

        /*
         * >> 1) a minus sign immediately after another operator is
         *       probably the sign for a negative number
         * >> 2) a minus sign at the beginning is also for a
         *       negative number
         * >> 3) a minus sign immediately after a parentheses is for a
         *       negative number as well
         */


        if (solv(&expr, &infix)) {
            printf("Synatx Error (#C1)!\n");
            continue;
        };


/* 7*8-(9/10) + 3 * (5-2*3) */

#ifdef SHOW_POSTFIX
        i = 0;
        printf("INFIX: ");
        while (i < linkedlist_size(&infix)) {
            token_t *t = linkedlist_get(&infix, i++);
            if (t->type == TOKEN_OPERATOR) {
                printf("%c ", t->op);
            } else {
                printf("%g ", t->num);
            }
        }
        printf("\n");
#endif

        if (inToPostFix(&infix, &postfix)) {
            printf("Synatx Error (#C2)!\n");
            continue;
        };

#ifdef SHOW_POSTFIX
        i = 0;
        printf("POSTFIX: ");
        while (i < linkedlist_size(&postfix)) {
            token_t *t = linkedlist_get(&postfix, i++);
            if (t->type == TOKEN_OPERATOR) {
                printf("%c ", t->op);
            } else {
                printf("%g ", t->num);
            }
        }
        printf("\n");
#endif

        if (eval(&postfix, &val)) {
            printf("Synatx Error (#C3)!\n");
        } else {
            printf("\t%d\n", (int) val);
        }
    }

    return 0;

}
