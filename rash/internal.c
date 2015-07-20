#include <stdio.h>
#include <stdlib.h>
#include "rash.h"

void echo(char *argv[]) {

    /* reprint arguments */
    while (*++argv)
        printf("%s ", *argv);
    printf("\n");

}

void quit(char *argv[]) {

    /* exit the shell */
    if (argv[1]) {
        /* invalid number of arguments */
        fprintf(stderr, "Error: Invalid arguments.\n");
        fprintf(stderr, "Usage: %s\n", argv[0]);
    } else {
        log_close();
        exit(0);
    }

}

void cd(char *argv[]) {

    /* change directory */
    if (!argv[1] || (argv[1] && argv[2])) {
        /* invalid number of arguments */
        fprintf(stderr, "Error: Invalid arguments.\n");
        fprintf(stderr, "Usage: %s dir_path\n", argv[0]);
    } else {
        if (do_chdir(argv[1]))
            fprintf(stderr, "Not a directory!\n");
    }

}

void history(char *argv[]) {

    /* print rash version */
    if (argv[1]) {
        /* invalid number of arguments */
        fprintf(stderr, "Error: Invalid arguments.\n");
        fprintf(stderr, "Usage: %s\n", argv[0]);
    } else {
        hist_t *ptr = get_history_first();
        int count = 1;
        while(ptr) {
            printf(" %d %s", count++, ptr->cmd);
            ptr = ptr->next;
        }
    }

}

void version(char *argv[]) {

    /* print rash version */
    if (argv[1]) {
        /* invalid number of arguments */
        fprintf(stderr, "Error: Invalid arguments.\n");
        fprintf(stderr, "Usage: %s\n", argv[0]);
    } else {
        printf("rash 2.0.1\n");
    }

}

void help(char *argv[]) {

    /* print help */
    if (argv[1]) {
        /* invalid number of arguments */
        fprintf(stderr, "Error: Invalid arguments.\n");
        fprintf(stderr, "Usage: %s\n", argv[0]);
    } else {
        printf("Available built-in commands:\n");
        printf("- echo\n");
        printf("- exit\n");
        printf("- quit\n");
        printf("- cd\n");
        printf("- chdir\n");
        printf("- history\n");
        printf("- version\n");
        printf("- help\n");
    }

}

void (*getfunc(char *str))(char *argv[]) {

    /* convert str into function pointer */
    if (!strcmp(str, "echo")) {
        return &echo;
    } else if (!strcmp(str, "exit") || !strcmp(str, "quit")) {
        return &quit;
    } else if (!strcmp(str, "cd") || !strcmp(str, "chdir")) {
        return cd;
    } else if (!strcmp(str, "history")) {
        return history;
    } else if (!strcmp(str, "version")) {
        return version;
    } else if (!strcmp(str, "help")) {
        return help;
    } else {
        return NULL;
    }

}
