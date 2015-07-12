#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "rash.h"

void proccmd(char *cmd, char **pathv) {

    /* process a command */
    char cmdpath[FILENAME_MAX+1];
    char **argv;
    void (*func)(char *argv[]);
    int wait = 1;
    int status;
    rash_pid_t pid;
    int i;

    /* return if comment */
    if (cmd[0] == '#')
        return;

    /* look for a trailing '&' character (and skip trailing spaces) */
    for (i = strlen(cmd)-1; i >= 0 && isspace(cmd[i]); i--);
    if (i >= 0 && cmd[i] == '&') {
        /* found a trailing & */
        cmd[i] = ' ';
        wait = 0;
    }

    /* tokenize cmd */
    argv = tokenize(cmd, " \n\t\r\v\f");
    if (argv == NULL) {
        return;
    }

    /* return if argv is too small */
    if (!argv[0]) {
        freev(argv);
        return;
    }

    /* execute command */
    if (func = getfunc(argv[0])) {
        /* internal function */
        func(argv);
    } else {
        /* not internal */
        if (pid = do_fork()) {
            /* parent, wait for child if no & in the command */
            if (wait)
                do_waitpid(pid, &status);
        } else {
            /* child, check if command contains a '/' */
            for (i = 0; argv[0][i] && argv[0][i] != '/'; i++);
            if (argv[0][i] == '/') {
                /* relative or absolute path */
                do_execv(argv[0], argv);
            } else {
                /* use $PATH environment variable */
                for (i = 0; pathv[i]; i++) {
                    strcpy(cmdpath, pathv[i]);
                    if (pathv[i][strlen(pathv[i])-1] != '/')
                        strcat(cmdpath, "/");
                    strcat(cmdpath, argv[0]);
                    if (do_exists(cmdpath)) /* try this path */ {
                        do_execv(cmdpath, argv);
                        break;
                    }
                }
            }
            /* cannot execute comamnd */
            fprintf(stderr, "%s: command not found.\n", argv[0]);
            exit(127);
        }
    }

    /* deallocate argv */
    freev(argv);

    /* done */
    return;

}
