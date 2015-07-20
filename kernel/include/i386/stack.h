/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> i386: stack header.                              | |
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

#ifndef STACK_H
#define STACK_H

#include <i386/protect.h>
#include <arch/type.h>

/* Registers & Stacks:  */
/* -------------------- */
#define store_reg()   \
__asm__("pusha    "); \
__asm__("pushw %gs"); \
__asm__("pushw %fs"); \
__asm__("pushw %es"); \
__asm__("pushw %ds"); \
__asm__("movw %%cx, %%ds; movw %%cx, %%es"::"c"(GDT_SEGMENT_SELECTOR(\
                                                GDT_ENTRY_KERNEL_DATA)));

#define restore_reg() \
__asm__("popw  %ds"); \
__asm__("popw  %es"); \
__asm__("popw  %fs"); \
__asm__("popw  %gs"); \
__asm__("popa");

/*   Stack style after an interrupt has been triggered:
 *
 *            (stack top)
 *           +-----------+
 *           |    ds     |
 *           +-----------+
 *           |    es     |
 *           +-----------+
 *           |    fs     |
 *           +-----------+
 *           |    gs     |
 *           +-----------+
 *
 *           +-----------+
 *           |    edi    |
 *           +-----------+
 *           |    esi    |
 *           +-----------+
 *           |    ebp    |
 *           +-----------+
 *           |    esp    |
 *           +-----------+
 *           |    ebx    |
 *           +-----------+
 *           |    edx    |
 *           +-----------+
 *           |    ecx    |
 *           +-----------+
 *           |    eax    |
 *           +-----------+
 *
 *           +-----------+
 *           |    err    |
 *           +-----------+
 *
 *           +-----------+
 *           |    eip    |
 *           +-----------+
 *           |    cs     |
 *           +-----------+
 *           |  eflags   |
 *           +-----------+
 *           | umode esp |
 *           +-----------+
 *           | umode ss  |
 *           +-----------+
 *
 */

typedef struct {
    uint16_t ds;
    uint16_t es;
    uint16_t fs;
    uint16_t gs;

    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t err;

    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t umode_esp;
    uint32_t umode_ss;
} Regs;

#define get_regs() (__extension__({                             \
    Regs __regs;                                                \
    __asm__("2:");                                              \
                                                                \
    __asm__("mov %%eax,   %%eax":"=a"(__regs.eax   ));          \
    __asm__("mov %%ebx,   %%eax":"=a"(__regs.ebx   ));          \
    __asm__("mov %%ecx,   %%eax":"=a"(__regs.ecx   ));          \
    __asm__("mov %%edx,   %%eax":"=a"(__regs.edx   ));          \
    __asm__("mov %%esp,   %%eax":"=a"(__regs.esp   ));          \
    __asm__("mov %%ebp,   %%eax":"=a"(__regs.ebp   ));          \
    __asm__("mov %%esi,   %%eax":"=a"(__regs.esi   ));          \
    __asm__("mov %%edi,   %%eax":"=a"(__regs.edi   ));          \
                                                                \
                                                                \
    __asm__("mov $2b,     %%eax":"=a"(__regs.eip   ));          \
    __asm__("pushfl; popl %%eax":"=a"(__regs.eflags));          \
                                                                \
    __regs.cs = GDT_SEGMENT_SELECTOR(GDT_ENTRY_KERNEL_CODE);    \
    __regs.ds = GDT_SEGMENT_SELECTOR(GDT_ENTRY_KERNEL_DATA);    \
    __regs.es = GDT_SEGMENT_SELECTOR(GDT_ENTRY_KERNEL_DATA);    \
    __regs.fs = GDT_SEGMENT_SELECTOR(GDT_ENTRY_KERNEL_DATA);    \
    __regs.gs = GDT_SEGMENT_SELECTOR(GDT_ENTRY_KERNEL_DATA);    \
                                                                \
    __regs.err = 0;                                             \
    __regs.esp -= 16;                                           \
                                                                \
    &__regs;                                                    \
}))

/* Stack: */
#define USER_STACK_SIZE         0x1000000 /* 16MB       */
#define KERNEL_STACK_SIZE       8*1024    /* 8KB Stack. */
extern uint8_t kernel_stack[];

#define stack_switch() \
    __asm__ ("mov %%eax, %%esp"::"a"(&kernel_stack[KERNEL_STACK_SIZE]));

#endif
