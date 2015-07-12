/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> i386: assembly header                            | |
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

#ifndef ASM_H
#define ASM_H

#include <arch/type.h>

#define code16() __asm__(".code16gcc")
#define code32() __asm__(".code32")

#define cli()   __asm__ ("cli\n\t")
#define sti()   __asm__ ("sti\n\t")

#define halt()  __asm__ ("cli;hlt\n\t")

#define iret()  __asm__ ("iret\n\t")

#define nop()   __asm__ ("nop\n\t")

#define call(addr) __asm__("call *%%eax"::"a"(addr));

#define inb(port) (__extension__({                      \
            uint8_t __res;                              \
            __asm__ ("inb  %%dx, %%al\n\t":"=a"(__res)  \
                                          :"d"(port));  \
            __res;                                      \
        }))

#define outb(value, port) \
            __asm__ ("outb %%al, %%dx\n\t"::"a"(value), "d"(port))

#define inw(port) (__extension__({                      \
            uint16_t __res;                             \
            __asm__ ("inw  %%dx, %%ax\n\t":"=a"(__res)  \
                                          :"d"(port));  \
            __res;                                      \
        }))

#define outw(value, port) \
            __asm__ ("outw %%ax, %%dx\n\t"::"a"(value), "d"(port))

#define inl(port) (__extension__({                      \
            uint32_t __res;                             \
            __asm__ ("inl  %%dx, %%eax\n\t":"=a"(__res) \
                                           :"d"(port)); \
            __res;                                      \
        }))

#define outl(value, port) \
            __asm__ ("outl %%eax,%%dx\n\t"::"a"(value), "d"(port))

#define insl(port, buf, nr) \
            __asm__ ("cld;rep;insl\n\t"::"d"(port), "D"(buf), "c"(nr))

#define outsl(buf, nr, port) \
            __asm__ ("cld;rep;outsl\n\t"::"d"(port), "S" (buf), "c" (nr))

#define in8_p(port) (__extension__({                    \
            uint8_t __res;                              \
            __asm__ ("inb  %%dx, %%al\n\t":"=a"(__res)  \
                                          :"dx"(port)); \
            __res;                                      \
        }))

#define out8_p(port, value) \
            __asm__ ("outb %%al, %%dx\n\t"::"al"(value), "dx"(port))

#define insl32(port, buf, nr) \
            __asm__ ("cld;rep;insl\n\t"::"d"(port), "D"(buf), "c"(nr))

#define outsl32(port, buf, nr) \
            __asm__ ("cld;rep;outsl\n\t"::"d"(port), "S" (buf), "c" (nr))


#define get_cr0() (__extension__({                      \
            uint32_t __res;                             \
            __asm__ ("mov %%cr0, %0":"=r"(__res));      \
            __res;                                      \
        }))

#define set_cr0(val) __asm__("mov %0, %%cr0"::"r"(val))

#define get_cr2() (__extension__({                      \
            uint32_t __res;                             \
            __asm__ ("mov %%cr2, %0":"=r"(__res));      \
            __res;                                      \
        }))

#define set_cr2(val) __asm__("mov %0, %%cr2"::"r"(val))

#define get_cr3() (__extension__({                      \
            uint32_t __res;                             \
            __asm__ ("mov %%cr3, %0":"=r"(__res));      \
            __res;                                      \
        }))

#define set_cr3(val) __asm__("mov %0, %%cr3"::"r"(val))

#define get_eflags() (__extension__({                   \
            uint32_t __res;                             \
            __asm__ ("pushfl; pop %0":"=r"(__res));     \
            __res;                                      \
        }))

#define set_eflags(val) __asm__("pushl %0; popfl"::"r"(val))

#define lgdt(gdtr)  __asm__("lgdt %0":"=m"(gdtr))
#define lidt(idtr)  __asm__("lidt %0":"=m"(idtr))

#define set_cs(val) __asm__("jmpl %0,$1f;1:"::"i"(val))
#define set_ds(val) __asm__("mov %%ax, %%ds"::"a"(val))
#define set_es(val) __asm__("mov %%ax, %%es"::"a"(val))
#define set_fs(val) __asm__("mov %%ax, %%fs"::"a"(val))
#define set_gs(val) __asm__("mov %%ax, %%gs"::"a"(val))
#define set_ss(val) __asm__("mov %%ax, %%ss"::"a"(val))

#define get_esp() (__extension__({                      \
            uint32_t __res;                             \
            __asm__("mov %%esp, %%eax":"=a"(__res):);   \
            __res;                                      \
        }))

#define set_esp(val) __asm__("mov %%eax,%%esp; nop"::"a"(val))

#define get_ebp() (__extension__({                      \
            uint32_t __res;                             \
            __asm__("mov %%ebp, %%eax":"=a"(__res):);   \
            __res;                                      \
        }))

#define set_ebp(val) __asm__("mov %%eax,%%ebp; nop"::"a"(val))

#endif
