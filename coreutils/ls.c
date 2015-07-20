/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Core Utilities.                             | |
 *        | |  -> ls.                                              | |
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
#include <errno.h>
#include <stdlib.h>
#include <sys/fs.h>
#include <tty/vtty.h>

typedef struct lsentry {
    struct lsentry *next;
    char* type;
    int   isFile;
    pos_t size;
    ino_t ino;
    char* name;
} lsentry_t;

lsentry_t *lshead = NULL;
lsentry_t *lstail = NULL;

void setAttr(unsigned char data) {
    ioctl(1 /*stdout*/, TTY_ATTR, (void *) &data);
}

int digits(pos_t num) {

    int count = 1;
    while (num /= 10)
        count++;
    return count;

}

int main(int argc, char *argv[], char *envp[]) {

    int fd, err;
    dirent_t dir;
    pos_t max_size = 0;
    ino_t max_ino = 0;
    stat_t st = {0};
    lsentry_t *p, *entry, *next;
    int size_digits;
    int ino_digits;

    if (argc > 1) {
        err = chdir(argv[1]);
        if (err) {
            printf("error: can't access the directory.\n");
            return -1;
        }
    }

    fd = open(".", 0);

    if (fd < 0) {
        printf("error: can't open the directory.\n");
        return -1;
    }

    while(1) {
        if (!readdir(fd, &dir))
            break; /* done. */

        /* stat. */
        if (stat(dir.name, &st)) {
            printf("can't stat %d\n", dir.name);
        } else {

            entry = malloc(sizeof(lsentry_t));

            entry->next = NULL;

            switch (st.mode & FT_MASK) {
                case FT_DIR:
                entry->type = "<DIR>";
                entry->isFile = 0;
                break;

                case FT_REGULAR:
                entry->type = "<REG>";
                entry->isFile = 1;
                break;

                case FT_SPECIAL:
                entry->type = "<DEV>";
                entry->isFile = 0;
                break;

                default:
                entry->type = "<NIL>";
                entry->isFile = 0;
            }

            entry->size = st.size;
            if (entry->size > max_size)
                max_size = entry->size;

            entry->ino  = st.ino;
            if (entry->ino > max_ino)
                max_ino = entry->ino;

            entry->name = malloc(strlen(dir.name)+1);
            strcpy(entry->name, dir.name);

            if (lstail) {
                lstail->next = entry;
                lstail = entry;
            } else {
                lshead = entry;
                lstail = entry;
            }

        }
    }

    p = lshead;
    size_digits = digits(max_size);
    ino_digits  = digits(max_ino);

    while(p) {

        printf(" %s ", p->type);
        /*printf("(%*u) ", ino_digits, p->ino);*/
        if (p->isFile) {
            printf("%*llu ", size_digits, p->size);
        } else {
            int i = size_digits+1;
            while(i--)
                printf(" ");
        }

        setAttr(0x0E);
        printf("%s\n", p->name);
        setAttr(0x0F);

        next = p->next;
        free(p->name);
        free(p);
        p = next;

    }

    close(fd);
    return 0;
}
