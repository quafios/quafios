/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> i386: protected mode.                            | |
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

#include <i386/asm.h>
#include <i386/protect.h>
#include <i386/stack.h>
#include <i386/page.h>

/* GDT Style:
 * 0x00 - 0x0000000000000000 -> Free.
 * 0x08 - 0xXX4X89XXXXXXXXXX -> TSS.
 * 0x10 - 0x00CF9A000000FFFF -> 0x00000000 : 0xFFFFFFFF kernel CS.
 * 0x18 - 0x00CF92000000FFFF -> 0x00000000 : 0xFFFFFFFF kernel DS.
 * 0x20 - 0x00CFFA000000FFFF -> 0x00000000 : 0xFFFFFFFF User-mode CS.
 * 0x28 - 0x00CFF2000000FFFF -> 0x00000000 : 0xFFFFFFFF User-mode DS.
 */

/* GDT: */
struct segment_descriptor gdt[GDT_ENTRY_COUNT] __attribute__
    ((aligned(sizeof(struct segment_descriptor)))) = {
        {0,0,0,0,0,0,0,0},
        {0,0,GDT_ETYPE_TS,PL_KERNEL,GDT_PRESENT,0,GDT_Flags_TS,0},
        {GDT_LIMIT_LO_DEFAULT, GDT_BASE_LO_DEFAULT, GDT_ETYPE_CODE,
         PL_KERNEL, GDT_PRESENT, GDT_LIMIT_HI_DEFAULT,
         GDT_Flags_DEFAULT, GDT_BASE_HI_DEFAULT},
        {GDT_LIMIT_LO_DEFAULT, GDT_BASE_LO_DEFAULT, GDT_ETYPE_DATA,
         PL_KERNEL, GDT_PRESENT, GDT_LIMIT_HI_DEFAULT,
         GDT_Flags_DEFAULT, GDT_BASE_HI_DEFAULT},
        {GDT_LIMIT_LO_DEFAULT, GDT_BASE_LO_DEFAULT, GDT_ETYPE_CODE,
         PL_USER, GDT_PRESENT, GDT_LIMIT_HI_DEFAULT,
         GDT_Flags_DEFAULT, GDT_BASE_HI_DEFAULT},
        {GDT_LIMIT_LO_DEFAULT, GDT_BASE_LO_DEFAULT, GDT_ETYPE_DATA,
         PL_USER, GDT_PRESENT, GDT_LIMIT_HI_DEFAULT,
         GDT_Flags_DEFAULT, GDT_BASE_HI_DEFAULT}
    };

/* GDT Register [GDTR]: */
struct {
    uint16_t limit;
    uint32_t address;
} __attribute__ ((packed)) gdtr __attribute__((aligned(8)));

/* IDT: */
struct gate_descriptor idt[IDT_ENTRY_COUNT] __attribute__
    ((aligned(sizeof(struct gate_descriptor)))) = {0};

/* IDT Register [IDTR]: */
struct {
    uint16_t limit;
    uint32_t address;
} __attribute__ ((packed)) idtr __attribute__((aligned(8)));

/* TS: */
struct task_struct ts __attribute__ ((aligned(8)));

void gdt_init() {

    /* Initialize GDT Register [GDTR]. */
    gdtr.limit = sizeof(gdt) - 1;
    gdtr.address = (uint32_t) gdt;

    /* Load GDTR. */
    lgdt(gdtr);

    /* Initialize segment selectors [CS, DS, ES, FS, GS, SS]: */
    __asm__("jmpl %0, $1f;1:"
            ::"i"(GDT_SEGMENT_SELECTOR(GDT_ENTRY_KERNEL_CODE)));
    __asm__("movw %%ax, %%ds"
            ::"a"(GDT_SEGMENT_SELECTOR(GDT_ENTRY_KERNEL_DATA)));
    __asm__("movw %%ax, %%es"
            ::"a"(GDT_SEGMENT_SELECTOR(GDT_ENTRY_KERNEL_DATA)));
    __asm__("movw %%ax, %%fs"
            ::"a"(GDT_SEGMENT_SELECTOR(GDT_ENTRY_KERNEL_DATA)));
    __asm__("movw %%ax, %%gs"
            ::"a"(GDT_SEGMENT_SELECTOR(GDT_ENTRY_KERNEL_DATA)));
    __asm__("movw %%ax, %%ss"
            ::"a"(GDT_SEGMENT_SELECTOR(GDT_ENTRY_KERNEL_DATA)));

    /* a NOP is good. */
    nop();

}

void idt_init() {

    /* Initialize IDT Register [IDTR]. */
    idtr.limit = sizeof(idt) - 1;
    idtr.address = (uint32_t) idt;

    /* Load IDTR. */
    __asm__("lidt %0":"=m"(idtr));

}

void ts_init() {

    /* Set TS struct size and base in GDT. */
    gdt[GDT_ENTRY_TS].limit_lo = ((sizeof(ts)-1) & 0x0FFFF)>> 0;
    gdt[GDT_ENTRY_TS].limit_hi = ((sizeof(ts)-1) & 0xF0000)>>16;
    gdt[GDT_ENTRY_TS].base_lo  = (((uint32_t) &ts) & 0x00FFFFFF)>> 0;
    gdt[GDT_ENTRY_TS].base_hi  = (((uint32_t) &ts) & 0xFF000000)>>24;

    /* Set critical values in task segment: */
    /* kernel-mode stack... */
    ts.esp0 = (uint32_t) &kernel_stack[KERNEL_STACK_SIZE];
    ts.ss0  = GDT_SEGMENT_SELECTOR(GDT_ENTRY_KERNEL_DATA);

    /* Load Task Register [TR] */
    __asm__("ltrw %%ax"::"a"(GDT_SEGMENT_SELECTOR(GDT_ENTRY_TS)));

}

void protect_init() {
    gdt_init();
    idt_init();
    ts_init();
}
