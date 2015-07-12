#ifndef RASH_H
#define RASH_H

#include <stdio.h>

/* macros */
#define MAX_CMD_LINE    513
#define MAX_CMD         4096

/* colors */
#define COLOR_WHITE     0
#define COLOR_RED       1
#define COLOR_YELLOW    2
#define COLOR_GREEN     3
#define COLOR_CYAN      4
#define COLOR_BLUE      5
#define COLOR_GREY      6
#define COLOR_BLACK     7

/* types */
typedef int rash_pid_t;
typedef struct hist {
    struct hist *next;
    char *cmd;
} hist_t;

/* function prototypes */
char **tokenize(char *str, char *del);
void freev(char **strv);
void (*getfunc(char *str))(char *argv[]);
void readcmd(FILE *in, char **pathv);
void proccmd(char *cmd, char **pathv);
void add_to_history(char *cmd);
hist_t *get_history_first();
int log_init();
void log_close();
void handler(int sig);

/* OS-specfic */
rash_pid_t do_fork();
int do_execv(char *path, char *argv[]);
int do_exists(char *path);
rash_pid_t do_wait(int *stat);
rash_pid_t do_waitpid(rash_pid_t pid, int *stat);
void do_getcwd(char *cwd, int size);
int do_chdir(char *dir);
void do_setcolor(int color);

#endif

