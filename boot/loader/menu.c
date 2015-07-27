/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Boot-Loader.                                | |
 *        | |  -> Menu view.                                       | |
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

#include <arch/type.h>

char header[] = {
0x20, 0x20, 0x20, 0x5F, 0x5F, 0x5F, 0x5F, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5F, 0x5F, 0x20, 0x5F, 0x20,
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x0A, 0x20, 0x20,
0x2F, 0x5F, 0x5F, 0x5F, 0x20, 0x5C, 0x5F, 0x20, 0x20, 0x20, 0x5F, 0x20, 0x20,
0x5F, 0x5F, 0x20, 0x5F, 0x20, 0x2F, 0x20, 0x5F, 0x28, 0x5F, 0x29, 0x20, 0x5F,
0x5F, 0x5F, 0x20, 0x20, 0x5F, 0x5F, 0x5F, 0x20, 0x0A, 0x20, 0x2F, 0x2F, 0x20,
0x20, 0x2F, 0x20, 0x2F, 0x20, 0x7C, 0x20, 0x7C, 0x20, 0x7C, 0x2F, 0x20, 0x5F,
0x60, 0x20, 0x7C, 0x20, 0x7C, 0x5F, 0x7C, 0x20, 0x7C, 0x2F, 0x20, 0x5F, 0x20,
0x5C, 0x2F, 0x20, 0x5F, 0x5F, 0x7C, 0x0A, 0x2F, 0x20, 0x5C, 0x5F, 0x2F, 0x20,
0x2F, 0x7C, 0x20, 0x7C, 0x5F, 0x7C, 0x20, 0x7C, 0x20, 0x28, 0x5F, 0x7C, 0x20,
0x7C, 0x20, 0x20, 0x5F, 0x7C, 0x20, 0x7C, 0x20, 0x28, 0x5F, 0x29, 0x20, 0x5C,
0x5F, 0x5F, 0x20, 0x5C, 0x0A, 0x5C, 0x5F, 0x5F, 0x5F, 0x2C, 0x5F, 0x5C, 0x20,
0x5C, 0x5F, 0x5F, 0x2C, 0x5F, 0x7C, 0x5C, 0x5F, 0x5F, 0x2C, 0x5F, 0x7C, 0x5F,
0x7C, 0x20, 0x7C, 0x5F, 0x7C, 0x5C, 0x5F, 0x5F, 0x5F, 0x2F, 0x7C, 0x5F, 0x5F,
0x5F, 0x2F, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
};

int32_t first_line, menu_height;

char *options[] = {"Boot Quafios 2.0.1 in text mode",
                   "Boot Quafios 2.0.1 in graphics mode",
                   "Boot from first hard disk",
                   "Reboot"};

int32_t option_count = 4;
int32_t selected_option = 0;

uint8_t border_attr = 0x0C;

extern uint8_t enable_graphics;

int32_t strlen(char *str) {
    int32_t i = 0;
    while(str[i])
        i++;
    return i;
}

int32_t linelen(char *str) {
    int32_t i = 0;
    while(str[i] != '\n')
        i++;
    return i;
}
char *getline(char *str, int32_t line) {
    int32_t i = 0;
    while (i++ < line)
        str = &str[linelen(str)];
    return str;
}

void print_header(int32_t line) {
    int32_t i, j, line_width, k = 0;
    line_width = linelen(header);
    for (j = 0; header[k]; j++) {
        cursor(0, line+j);
        printf("%a", 0x0E);
        for (i = 0; i < (80-line_width-1)/2; i++)
            printf(" ");
        for (i = 0; i < line_width; i++)
            printf("%c", header[k++]);
        k++;
    }
    first_line = ++j;
}

void print_menu_header(int32_t line) {
    int32_t i;
    cursor(0, line);
    printf("%a", border_attr);
    printf(" %c", 218);
    for (i = 0; i < 76; i++)
        printf("%c", 196);
    printf("%c ", 191);
}

void print_menu_footer(int32_t line) {
    int32_t i;
    cursor(0, line);
    printf("%a", border_attr);
    printf(" %c", 192);
    for (i = 0; i < 76; i++)
        printf("%c", 196);
    printf("%c ", 217);
}

void print_footer(int32_t line) {
    int32_t i;
    cursor(0, line);
    printf("%a", 0x0F);
    printf("     Thank you for using Quafios. "
              "Quafios is free software; any user has\n"
           "   the freedom to run, copy, distribute, study, change and "
              "improve Quafios.\n"
           "        For more information, please visit "
              "(http://www.quafios.com/).");
}

void print_menu_option(int32_t line, char *str, int32_t selected) {
    int32_t i;
    cursor(0, line);
    printf("%a", border_attr);
    printf(" %c", 179);

    if (selected)
        printf("%a", 0x2F);
    else
        printf("%a", 0x0F);

    printf(" %s", str);

    for (i = 0; i < 75-strlen(str); i++)
        printf(" ");

    printf("%a", border_attr);
    printf("%c ", 179);
}

void show_menu() {
    int32_t i;
    cls();
    hide_cursor();
    print_header(1);
    menu_height = 18-first_line;
    print_menu_header(first_line++);
    for (i = 0; i < option_count; i++)
        print_menu_option(first_line+i, options[i], selected_option == i);
    for (; i < menu_height; i++)
        print_menu_option(first_line+i, "", 0);
    print_menu_footer(first_line+i++);
    print_footer(first_line+i+1);
    while(1) {
        uint8_t but = getc();
        switch (but) {
            case 72:
                /* up */
                if (selected_option) {
                    selected_option--;
                    print_menu_option(first_line+selected_option,
                                      options[selected_option], 1);
                    print_menu_option(first_line+selected_option+1,
                                      options[selected_option+1], 0);
                } else {
                    /* invalid */
                    beep();
                }
                break;
            case 80:
                if (selected_option < option_count-1) {
                    selected_option++;
                    print_menu_option(first_line+selected_option,
                                      options[selected_option], 1);
                    print_menu_option(first_line+selected_option-1,
                                      options[selected_option-1], 0);
                } else {
                    /* invalid */
                    beep();
                }
                /* down */
                break;
            case 28:
                /* return */
                switch (selected_option) {
                    case 0:
                        enable_graphics = 0;
                        return;
                    case 1:
                        enable_graphics = 1;
                        return;
                    case 2:
                        cls();
                        __asm__("mov  $0x02, %ah;"
                                "mov  $0x01, %al;"
                                "mov  $0x00, %ch;"
                                "mov  $0x00, %dh;"
                                "mov  $0x01, %cl;"
                                "mov  $0x80, %dl;"
                                "mov  $0x7C00, %bx;"
                                "int  $0x13;"
                                "jmp  $0x0000, $0x7C00");
                        return;
                    case 3:
                        cls();
                        __asm__("jmp $0xFFFF, $0x0000");
                        return;
                }
                break;
            default:
                /* invalid */
                beep();
                break;
        }
    }
}
