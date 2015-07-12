#include <stdlib.h>
#include <string.h>
#include "rash.h"

int count = 0;
hist_t *first = NULL;
hist_t *last = NULL;

void add_to_history(char *cmd) {
    hist_t *node = malloc(sizeof(hist_t));
    node->cmd  = strcpy(malloc(strlen(cmd)+1), cmd);
    node->next = NULL;
    if (last) {
        last->next = node;
        last = node;
    } else {
        first = node;
        last = node;
    }
    count++;
}

hist_t *get_history_first() {
    return first;
}
