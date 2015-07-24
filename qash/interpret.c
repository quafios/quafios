/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Shell.                                      | |
 *        | |  -> Command line interpretation.                     | |
 *        | +------------------------------------------------------+ |
 *        +----------------------------------------------------------+
 *
 * This file is part of Quafios 2.0.1 source code.
 * Copyright (C) 2015  Mostafa Abd El-Aziz Mohamed.
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
#include <sys/proc.h>

#include "io.h"
#include "func.h"

#define EXEC_NONE       0    /* nothing to do here */
#define EXEC_INTERNAL   1    /* internal function  */
#define EXEC_PROGRAM    2    /* external program   */

int argc;
char *argv[MAX_CMD];
char *path[] = {"/bin", NULL};
char exepath[FILENAME_MAX+1];

void interpret(void *cmd) {

    int i = 0;
    int err = 0;
    int etype;     /* execution type.      */
    int (*eptr)(); /* what to be executed. */
    char *tok;

    /* I: Tokenize the command "cmd" & create argv:  */
    /* --------------------------------------------- */
    tok = strtok(cmd, " ");
    while(1) {
        argc = i;
        argv[i++] = tok;
        if (tok == NULL)
            break;
        tok = strtok(NULL, " ");
    }

    /* II: Detect Execution Type:  */
    /* --------------------------- */
    if (argv[0] == NULL) {
        etype = EXEC_NONE;
    } else if ((!strcmp(argv[0], "exit")) || (!strcmp(argv[0], "quit"))) {
        etype = EXEC_INTERNAL;
        eptr  = shexit;
    } else if (!strcmp(argv[0], "help")) {
        etype = EXEC_INTERNAL;
        eptr  = help;
    } else if (!strcmp(argv[0], "version")) {
        etype = EXEC_INTERNAL;
        eptr  = version;
    } else if (!strcmp(argv[0], "pwd")) {
        etype = EXEC_INTERNAL;
        eptr  = pwd;
    } else if (!strcmp(argv[0], "cd")) {
        etype = EXEC_INTERNAL;
        eptr  = cd;
    } else if (!strcmp(argv[0], "echo")) {
        etype = EXEC_INTERNAL;
        eptr  = echo;
    } else {
        etype = EXEC_PROGRAM;
    }

    /* III: Fork the shell if it needs fork:  */
    /* -------------------------------------- */
    if (etype == EXEC_PROGRAM) {

        /* do the amazing fantastic fork! */
        int pid = fork();

        if (pid) {
            /* I am the parent... */

            /* sleep */
            int status;
            waitpid(pid, &status);

            /* done! let's return! */
            return;
        }
    }

    /* IV: Execute the command:  */
    /* ------------------------- */
    /* TODO: interpret |, >, < and &> things. */

    /* EXECUTE: */
    switch(etype) {
        case EXEC_NONE:
        break;

        case EXEC_INTERNAL:
        ((void (*)(void)) eptr)();
        break;

        case EXEC_PROGRAM:
        /* loop on all directories of "path" */
        i = 0;
        while(path[i] != NULL) {
            int len1 = strlen(path[i]);
            int len2 = strlen(argv[0]);
            strcpy(&exepath[0], path[i]);
            exepath[len1] = '/';
            strcpy(&exepath[len1+1], argv[0]);
            exepath[len1+1+len2] = 0;

            err = execve(exepath, argv, NULL);
            i++;
        }
        err = 1;
        break;
    }

    /* TODO: undo |, >, < and &> things. */

    /* check err: */
    if (err)
        printf("Command not found!\n");

    /* done. */
    if (etype == EXEC_PROGRAM) {
        _exit(err); /* we must exit because of the fork. */
    } else {
        return;
    }
}
