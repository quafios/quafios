/* Quafios-specific routines */

#include <stdio.h>
#include "rash.h"

#ifdef BUILD_FOR_QUAFIOS

#include <api/fs.h>
#include <api/proc.h>
#include <video/generic.h>
#include <tty/vtty.h>

rash_pid_t do_fork() {
    return fork();
}

int do_execv(char *path, char *argv[]) {
    return execve(path, argv, NULL);
}

void do_getcwd(char *cwd, int size) {
    getcwd(cwd, size);
}

int do_chdir(char *dir) {
    return chdir(dir);
}

rash_pid_t do_waitpid(rash_pid_t pid, int *status) {
    return waitpid(pid, status);
}

int do_exists(char *path) {
    stat_t st = {0};
    if (stat(path, &st)) {
        return 0;
    } else {
        return 1;
    }
}

void do_setcolor(int color) {

    unsigned char data = VGA_BG_BLACK;

    switch(color) {
        case COLOR_WHITE:
            data |= VGA_FG_WHITE;
            break;
        case COLOR_RED:
            data |= VGA_FG_RED_BRIGHT;
            break;
        case COLOR_YELLOW:
            data |= VGA_FG_YELLOW;
            break;
        case COLOR_GREEN:
            data |= VGA_FG_GREEN;
            break;
        case COLOR_CYAN:
            data |= VGA_FG_CYAN;
            break;
        case COLOR_BLUE:
            data |= VGA_FG_BLUE;
            break;
        case COLOR_GREY:
            data |= VGA_FG_WHITE;
            break;
        case COLOR_BLACK:
            data |= VGA_FG_BLACK;
            break;
        default:
            break;

    }

    ioctl(1 /*stdout*/, TTY_ATTR, (void *) &data);

}

#endif
