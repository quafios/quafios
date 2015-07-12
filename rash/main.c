#include <stdio.h>
#include <stdlib.h>
#include "rash.h"

int main(int argc, char *argv[]) {

    /* rash entry point */
    int ret = 0; /* return value */
    char **pathv = tokenize(getenv("PATH"), ":"); /* parse $PATH */

    /* initialize logger */
    if (log_init())
        return -3;

    /* run shell */
    if (argc < 2) {
        /* interactive mode */
        readcmd(stdin, pathv);
    } else if (argc == 2) {
        /* batch mode */
        FILE *bf = fopen(argv[1], "r"); /* open batch file */
        if (bf == NULL) {
            fprintf(stderr, "Error: Can't open %s.\n", argv[1]);
            return -2;
        }
        readcmd(bf, pathv);
        fclose(bf);
    } else {
        /* invalid arguments */
        fprintf(stderr, "Error: Invalid arguments.\n");
        fprintf(stderr, "Usage: %s [batch_file]\n", argv[0]);
        ret = -1;
    }

    /* free path vector */
    freev(pathv);

    /* done */
    return ret;

}
