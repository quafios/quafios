/* UNIX-specific routines */

#include <stdio.h>
#include "rash.h"

#ifdef BUILD_FOR_UNIX

#include <unistd.h>

rash_pid_t do_fork() {
    return fork();
}

int do_execv(char *path, char *argv[]) {
    return execv(path, argv);
}

void do_getcwd(char *cwd, int size) {
    getcwd(cwd, size);
}

void do_chdir(char *dir) {
    chdir(dir);
}

void do_waitpid(rash_pid_t pid) {
    int status;
    waitpid(pid, &status, 0);
}

void do_setcolor(int color) {

    switch(color) {
        case COLOR_WHITE:
            fputs("\e[1;37m", stdout);
            break;
        case COLOR_RED:
            fputs("\e[1;31m", stdout);
            break;
        case COLOR_YELLOW:
            fputs("\e[1;33m", stdout);
            break;
        case COLOR_GREEN:
            fputs("\e[1;32m", stdout);
            break;
        case COLOR_CYAN:
            fputs("\e[1;36m", stdout);
            break;
        case COLOR_BLUE:
            fputs("\e[1;34m", stdout);
            break;
        case COLOR_GREY:
            fputs("\e[0;37m", stdout);
            break;
        case COLOR_BLACK:
            fputs("\e[1;30m", stdout);
            break;
        default:
            break;

    }

}

#endif
