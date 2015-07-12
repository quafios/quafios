#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rash.h"

int isdel(char *del, char chr) {
    int i = 0;
    while (del[i]) {
        if (del[i++] == chr)
            return 1;
    }
    return 0;
}

char **tokenize(char *str, char *del) {

    /* tokenize string using del as a delimeter */
    char **tokens;      /* token vector to be returned */
    char *tmpptr;       /* temporary pointer */
    int  tok_i = 0;     /* index of current token */
    char tmp[MAX_CMD];  /* temporary buffer 1 */
    char tmp2[MAX_CMD]; /* temporary buffer 2 */
    char tmp3[MAX_CMD]; /* temporary buffer 3 */
    int  i = 0;         /* char counter */
    int  j = 0;         /* auxiliary counter */
    int  k = 0;         /* auxiliary counter 2 */
    int  l = 0;         /* auxiliary counter 3 */
    int  equ = 0;       /* equate character */
    int  count = 2;     /* occurences of del in str */

    /* calculate count of del in str */
    while(str[i])
        if (str[i++] == *del)
            count++;

    /* allocate tokens array */
    tokens = malloc(count*sizeof(char *));

    /* loop */
    i = 0;
    while(1) {
        /* skip whitspaces */
        while (isdel(del, str[i]))
            i++;
        /* end? */
        if (!str[i]) {
            tokens[tok_i] = NULL;
            break;
        }
        /* loop until we found another whitespace */
        j = 0;
        while (str[i] && !isdel(del, str[i])) {
            if (str[i] == '\"') {
                tmp[j++] = str[i++]; /* copy the quote */
                while (str[i] && str[i] != '\"')
                    tmp[j++] = str[i++]; /* copy all */
                if (!str[i]) {
                    fprintf(stderr, "Error: Invalid expression!\n");
                    tokens[tok_i] = NULL;
                    freev(tokens);
                    return NULL;
                }
                tmp[j++] = str[i++]; /* copy the quote */
            } else {
                tmp[j++] = str[i++];
            }
        }
        tmp[j] = 0;
        /* copy to tmp2 and evaluate variables */
        j = 0;
        k = 0;
        equ = -1;
        while(tmp[j]) {
            if (tmp[j] == '\"') {
                /* ignore quotes */
                j++;
            } else if (tmp[j] == '$'){
                /* variable */
                j++;
                l = 0;
                while((tmp[j] >= '0' && tmp[j] <= '9') ||
                      (tmp[j] >= 'A' && tmp[j] <= 'Z') ||
                      (tmp[j] >= 'a' && tmp[j] <= 'z'))
                    tmp3[l++] = tmp[j++];
                tmp3[l] = 0;
                if (strcmp(tmp3, "")) {
                    tmpptr = getenv(tmp3);
                    l = 0;
                    while(tmpptr && tmpptr[l])
                        tmp2[k++] = tmpptr[l++];
                } else {
                    tmp2[k++] = '$';
                }
            } else {
                /* ordinary char */
                if (tmp[j] == '=')
                    equ = k;
                tmp2[k++] = tmp[j++];
            }
        }
        tmp2[k] = 0;
        /* equate? */
        if (equ != -1) {
            if (equ == 0) {
                fprintf(stderr, "Error: Invalid expression!\n");
                tokens[tok_i] = NULL;
                freev(tokens);
                return NULL;
            }
            tmp2[equ] = 0;
            setenv(tmp2, &tmp2[equ+1], 1);
            continue;
        }
        /* copy token */
        strcpy(tokens[tok_i++]=malloc(strlen(tmp2)+1), tmp2);
    }

    /* done */
    return tokens;

}

void freev(char **strv) {

    int i = 0;
    while (strv[i])
        free(strv[i++]);
    free(strv);

}
