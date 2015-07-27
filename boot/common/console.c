/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Boot-Loader.                                | |
 *        | |  -> Console routines.                                | |
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
#include <sys/bootinfo.h>

uint8_t *vga   = (uint8_t *) 0xB8000;
uint8_t attrib = 0x0F;

uint32_t getX() {

    uint16_t pos;
    int32_t x, y;

    /* Get cursor position */
    __asm__("int $0x10":"=d"(pos):"a"(0x0300), "b"(0));

    /* Set x & y: */
    x = (pos>>0) & 0xFF; /* column (DL) */
    y = (pos>>8) & 0xFF; /* row    (DH) */

    /* return X */
    return x;

}

uint32_t getY() {

    uint16_t pos;
    int32_t x, y;

    /* Get cursor position */
    __asm__("int $0x10":"=d"(pos):"a"(0x0300), "b"(0));

    /* Set x & y: */
    x = (pos>>0) & 0xFF; /* column (DL) */
    y = (pos>>8) & 0xFF; /* row    (DH) */

    /* return Y */
    return y;

}

void cursor(int32_t x, int32_t y) {

    uint16_t pos = 0;

    /* Get x & y: */
    pos |= x<<0;  /* column (DL) */
    pos |= y<<8;  /* row    (DH) */

    /* Set cursor position */
    __asm__("int $0x10"::"a"(0x0200), "b"(0), "d"(pos));

}

void hide_cursor() {

    __asm__("int $0x10"::"a"(0x0100), "c"(0x2607));

}

void show_cursor() {

    __asm__("int $0x10"::"a"(0x0100), "c"(0x0607));

}

void cls() {
    int32_t i = 0;
    hide_cursor();
    attrib = 0x0F;
    while(i < 80*25*2) {
        vga[i++] = ' ';
        vga[i++] = attrib;
    }
    cursor(0, 0);
    show_cursor();
}

void scroll() {
    uint32_t i;
    for (i=0; i< 80 * 24 * 2; i++)
        vga[i] = vga[i + 80 * 2];

    /* clean last row: */
    for (; i < 80 * 25 * 2; i+=2)
        vga[i] = ' ';
}

void putc(char chr) {

    int32_t new_x, new_y;
    int32_t x, y;

    if (chr == '\n') {
        do putc(' '); while(getX());
        return;
    }

    x = getX();
    y = getY();

    vga[(y*80 + x)*2 + 0] = chr;
    vga[(y*80 + x)*2 + 1] = attrib;

    if (x != 79) {
        new_x = x+1;
        new_y = y;
    } else {
        new_x = 0;
        if (y != 24) {
            new_y = y+1;
        } else {
            scroll();
            new_y = y;
        }
    }

    cursor(new_x, new_y);
}

void printDec(uint32_t value) {
    uint32_t n = value / 10;
    int32_t r = value % 10;
    if (value >= 10) printDec(n);
    putc(r+'0');
}

void printHex(uint32_t value) {
    int32_t i;
    for(i = 7; i >= 0; i--)
        putc("0123456789ABCDEF"[(value>> (i*4)) & 0x0F]);
}

void printf(char *format, ...) {

    uint32_t arg_id = 0, i;
    int32_t  *addr = (int32_t *) &format;

    for (i = 0; format[i] != 0; i++)
        if (format[i] == '%')
            switch (format[++i]) {
                case 'a':
                    attrib = addr[++arg_id];
                    break;

                case 'c':
                    putc(addr[++arg_id]);
                    break;

                case 'd':
                    printDec(addr[++arg_id]);
                    break;

                case 'x':
                    printHex(addr[++arg_id]);
                    break;

                case 's':
                    printf((char *) addr[++arg_id]);
                    break;

                default:
                    break;
            }
        else
            putc(format[i]);

}

uint8_t getc() {
    uint16_t ret;
    __asm__("int $0x16":"=a"(ret):"a"(0x0000));
    return (ret>>8)&0xFF;
}

void beep() {

    /*
     * Reference: Mazidi.
     *
     * In IBM PC, CLK2 input of PIT (8253) is connected
     * to a frequency of 1.19318MHz. GATE2 input
     * is connected to PB0 of port 0x61 (kbc).
     * The OUT2 output of 8253 is NANDed with
     * PB1 of port 0x61, and the NANDed signal
     * is delivered to speaker driving circuit.
     *
     * We need to adjust the frequency divider such
     * that OUT2 has a square wave of 896Hz [beep sound].
     * Dividing the frequency 1.19318MHz by 896Hz gives 1331,
     * the value that will be loaded to counter 2.
     *
     * So, the control word of PIT will be 0xb6
     * D0: BCD (0)
     * D1-D3: mode (011)  [mode 3 is square wave]
     * D4-D5: Read/Load  (11: read/load LSB, then MSB)
     * D6-D7: select counter (10)
     */
    __asm__("mov $0xb6, %al   ;");
    __asm__("out %al,   $0x43 ;");
    /* now load counter 2 with 1331 as calculated above */
    __asm__("mov $1331,  %ax  ;");
    __asm__("out %al,   $0x42 ;");
    __asm__("mov %ah,   %al   ;");
    __asm__("out %al,   $0x42 ;");
    /* set PB0 and PB1 of kbc */
    __asm__("in  $0x61, %al   ;");
    __asm__("mov %al,   %ah   ;");
    __asm__("and $0x10, %ah   ;");
    __asm__("or  $0x03, %al   ;");
    __asm__("out %al,   $0x61 ;");
    /* PB4 of port 0x61 toggles every 15.085us. It can
     * be used to create processor-independent delays
     */
    __asm__("mov $33114,%cx   ;"); /* 33114*0.015085 = 500ms */
    __asm__("waitf:           ;");
    __asm__("in  $0x61, %al   ;");
    __asm__("and $0x10, %al   ;");
    __asm__("cmp %ah,   %al   ;");
    __asm__("je  waitf        ;"); /* toggled? */
    __asm__("mov %al,   %ah   ;"); /* store the new value */
    __asm__("loop waitf       ;"); /* store the new value */
    /* delay */
    __asm__("mov $400,   %bx  ;");
    __asm__("pause1:          ;");
    __asm__("mov 65535, %cx   ;");
    __asm__("pause2:          ;");
    __asm__("dec %cx          ;");
    __asm__("jne pause2       ;");
    __asm__("dec %bx          ;");
    __asm__("jne pause1       ;");
    /* clear PB0 and PB1 of kbc */
    __asm__("in  $0x61, %al   ;");
    __asm__("and $0xFC, %al   ;");
    __asm__("out %al,   $0x61 ;");

}
