/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> printk() procedure.                              | |
 *        | +------------------------------------------------------+ |
 *        +----------------------------------------------------------+
 *
 * This file is part of Quafios 1.0.2 source code.
 * Copyright (C) 2014  Mostafa Abd El-Aziz Mohamed.
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
#include <sys/printk.h>
#include <sys/device.h>
#include <sys/mm.h>
#include <tty/vtty.h>

device_t *system_console = NULL;  /* system console. */

void kputc(char c) {
    if (system_console == NULL)
        legacy_video_putc(c);
    else
        dev_write(system_console, (uint64_t) 0, 1, &c);
}

void kattr(char attr) {
    if (system_console == NULL)
        legacy_video_attr(attr);
    else
        dev_ioctl(system_console, TTY_ATTR, &attr);
}

void printk_parse_decimal(uint32_t value) {
    uint32_t n = value / 10;
    int32_t  r = value % 10;
    if (value >= 10) printk_parse_decimal(n);
    kputc(r+'0');
}

void printk_parse_hexa(uint32_t value) {
    int32_t i;
    for(i = 7; i >= 0; i--)
        kputc("0123456789ABCDEF"[(value>> (i*4)) & 0x0F]);
}

uint32_t printk_byaddr(uint32_t *addr) {

    uint32_t arg_id = 0, i;
    char *format = (char *) addr[arg_id];

    for (i = 0; format[i] != 0; i++)
        if (format[i] == '%')
            switch (format[++i]) {
                case 'c':
                    kputc(addr[++arg_id]);
                    break;

                case 'a':
                    kattr(addr[++arg_id]);
                    break;

                case 'd':
                    printk_parse_decimal(addr[++arg_id]);
                    break;

                case 'x':
                    printk_parse_hexa(addr[++arg_id]);
                    break;

                case 's':
                    arg_id+=printk_byaddr(&addr[++arg_id]);
                    break;

                default:
                    break;
            }
        else
            kputc(format[i]);

    return arg_id;
}

void printk(const char *format, ...) {
    uint32_t ebp;
    __asm__("mov %%ebp, %%eax":"=a"(ebp));
    printk_byaddr((uint32_t *) (ebp + 8));
}

