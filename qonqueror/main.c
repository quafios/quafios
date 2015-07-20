/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios File browser (Qonqueror).                   | |
 *        | |  -> main() procedure.                                | |
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
#include <gui.h>
#include <api/proc.h>

#define TYPE_AFILE       0
#define TYPE_BINFILE     1
#define TYPE_DEVFILE     2
#define TYPE_ELFFILE     3
#define TYPE_HFILE       4
#define TYPE_PNGFILE     5
#define TYPE_TXTFILE     6
#define TYPE_FOLDER      7

iconview_t *iv;

int tmpf = -100;

/***************************************************************************/
/*                              Linked Lists                               */
/***************************************************************************/

typedef struct dir_entry {
    struct lsentry *next;
    char* name;
    int   type;
} __attribute__((packed)) dir_entry_t;

dir_entry_t *firstent = NULL;

dir_entry_t *add_to_list(dir_entry_t *head, dir_entry_t *ent) {

}

/***************************************************************************/
/*                        Icons and MIME Types                             */
/***************************************************************************/

pixbuf_t *afile;
pixbuf_t *binfile;
pixbuf_t *devfile;
pixbuf_t *elffile;
pixbuf_t *hfile;
pixbuf_t *pngfile;
pixbuf_t *txtfile;
pixbuf_t *folder;

void load_icons() {

    afile   = parse_png("/usr/share/icons/afile.png");
    binfile = parse_png("/usr/share/icons/binfile.png");
    devfile = parse_png("/usr/share/icons/devfile.png");
    elffile = parse_png("/usr/share/icons/elffile.png");
    hfile   = parse_png("/usr/share/icons/hfile.png");
    pngfile = parse_png("/usr/share/icons/pngfile.png");
    txtfile = parse_png("/usr/share/icons/txtfile.png");
    folder  = parse_png("/usr/share/icons/folder.png");

}


pixbuf_t *get_icon(int type) {

    switch (type) {
        case TYPE_AFILE:
            return afile;
        case TYPE_BINFILE:
            return binfile;
        case TYPE_DEVFILE:
            return devfile;
        case TYPE_ELFFILE:
            return elffile;
        case TYPE_HFILE:
            return hfile;
        case TYPE_PNGFILE:
            return pngfile;
        case TYPE_TXTFILE:
            return txtfile;
        case TYPE_FOLDER:
            return folder;
    }

}

int ends_with(char *str, char *tail) {
    return !strcmp(&str[strlen(str)-strlen(tail)], tail);
}

int find_type(char *name) {

    stat_t st = {0};

    /* stat */
    stat(name, &st);

    /* determine type */
    switch (st.mode & FT_MASK) {
        case FT_DIR:
        return TYPE_FOLDER;

        case FT_REGULAR:
        /* find mime type */
        if (ends_with(name, ".a")) {
            /* library */
            return TYPE_AFILE;
        } else if (ends_with(name, ".bin")) {
            /* binary file */
            return TYPE_BINFILE;
        } else if (ends_with(name, ".h")) {
            /* C header */
            return TYPE_HFILE;
        } else if (ends_with(name, ".png")) {
            /* PNG file */
            return TYPE_PNGFILE;
        } else {
            /* ELF or text? */
            char elf_magic[] = {0x7F, 'E', 'L', 'F', 0};
            char buf[5] = {0};
            FILE *f = fopen(name, "r");
            if (f) {
                fread(buf, 4, 1, f);
                fclose(f);
                if (!strcmp(elf_magic, buf)) {
                    return TYPE_ELFFILE;
                } else {
                    return TYPE_TXTFILE;
                }
            } else {
                return TYPE_TXTFILE;
            }
        }

        case FT_SPECIAL:
        return TYPE_DEVFILE;
        break;

        default:
        return TYPE_TXTFILE;
    }

}

/***************************************************************************/
/*                            Directory Scan                               */
/***************************************************************************/

void open_dir(char *dirname) {

    int fd, type;
    dirent_t dir;

    /* clear current icons */
    iconview_reset(iv);

    /* clear the linked lists */
    /* TODO. */

    /* open the directory */
    chdir(dirname);
    fd = open(".", 0);

    /* loop over directory contents */
    while(1) {

        /* read next entry */
        if (!readdir(fd, &dir))
            break; /* done. */

        /* get the type of the entry */
        type = find_type(dir.name);

        /* add icon to iconview element */
        iconview_insert(iv, get_icon(type), dir.name);

    }

    /* redraw on the window */
    iconview_redraw(iv);

}

void launch(int selected) {

    char *fname = iv->icons[selected].title;
    int type = find_type(fname);
    char *argv[3] = {NULL};

    switch (type) {
        case TYPE_FOLDER:
        open_dir(fname);
        break;

        case TYPE_ELFFILE:
        argv[0] = fname;
        if (!fork()) {
            execve(fname, argv, NULL);
        }
        break;

        case TYPE_PNGFILE:
        argv[0] = "/bin/quafshot";
        argv[1] = fname;
        if (!fork()) {
            execve(argv[0], argv, NULL);
        }
        break;

        case TYPE_HFILE:
        case TYPE_TXTFILE:
        break;

        default:
        break;

    }

}

/***************************************************************************/
/*                              Main Routine                               */
/***************************************************************************/

int main() {

    window_t *win;

    /* allocate a window */
    win = window_alloc("Qonqueror", /* title      */
                       380,         /* width      */
                       250,         /* height     */
                       -2,          /* x (center) */
                       -2,          /* y (center) */
                       0xFFC0C0C0,  /* bg color   */
                       "/usr/share/icons/browser16.png" /* iconfile */);

    /* allocate iconview */
    iv = iconview_alloc(500, 48, 48,
                        win->pixbuf->width, win->pixbuf->height, 1);

    /* insert iconview */
    window_insert(win, iv, 0, 0);

    /* initialize events */
    iv->double_click = launch;

    /* load icons */
    load_icons();

    /* open directory */
    open_dir("/");

    tmpf = 1;
    /* loop */
    gui_loop();

    /* done */
    return 0;

}
