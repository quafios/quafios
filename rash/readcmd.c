#include <stdio.h>
#include <string.h>
#include "rash.h"

void readcmd(FILE *in, char **pathv) {

    /* read commands */
    char cmd[MAX_CMD_LINE+1] = {0}; /* command line */
    int i = -1; /* counter */
    while(1) {
        /* show prompt only if interactive mode */
        if (in == stdin) {
            /* echo shell name */
            do_setcolor(COLOR_RED);
            printf("rash");
            /* echo : */
            do_setcolor(COLOR_GREY);
            printf(":");
            /* echo current directory */
            do_setcolor(COLOR_YELLOW);
            do_getcwd(cmd, MAX_CMD_LINE);
#if 0
            for(i = strlen(cmd)-2; i >= 0 && cmd[i] != '/'; i--);
#endif
            printf(&cmd[i+1]);
            /* print prompt */
            do_setcolor(COLOR_RED);
            printf("$ ");
            /* set grey color */
            do_setcolor(COLOR_GREY);
        }
        /* read command line */
        if (!fgets(cmd, MAX_CMD_LINE, in)) {
            /* EOF */
            if (in == stdin)
                printf("\n");
            return;
        }
        if (strlen(cmd) == MAX_CMD_LINE-1 && !feof(in) &&
            cmd[strlen(cmd)-1] != '\n') {
            /* overflow */
            while (fgets(cmd, MAX_CMD_LINE, in) &&
                   cmd[strlen(cmd)-1] != '\n');
            fprintf(stderr, "Error: Command is too long.\n");
            continue;
        }
        /* print command to stdout if batch mode */
#if 0
        if (in != stdin) {
            printf(cmd);
        }
#endif
        /* add to command history */
        add_to_history(cmd);
        /* process/interpret the command */
        proccmd(cmd, pathv);
    }

}

