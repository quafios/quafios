/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Boot-Loader.                                | |
 *        | |  -> CPU mode controller.                             | |
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

uint64_t gdt[] __attribute__((aligned(8))) = {
    __extension__ 0x0000000000000000ULL, /* Null                        */
    __extension__ 0x008F9A000000FFFFULL, /* 16-bit Code Flat Descriptor */
    __extension__ 0x008F92000000FFFFULL, /* 16-bit Data Flat Descriptor */
    __extension__ 0x00CF9A000000FFFFULL, /* 32-bit Code Flat Descriptor */
    __extension__ 0x00CF92000000FFFFULL  /* 32-bit Data Flat Descriptor */
};

struct gdtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdtr __attribute__((aligned(8)));

struct idtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr __attribute__((aligned(8)));

void enter_unreal() {

    /* Keep safe & disable IRQs. */
    cli();

    /* Initialize GDTR structure: */
    gdtr.limit = sizeof(gdt)-1;
    gdtr.base  = (uint64_t) &gdt;

    /* Set LGDTR */
    lgdt(gdtr);

    /* Enter Protected Mode: */
    set_cr0(get_cr0() | 0x00000001);

    /* Load DS & ES: */
    set_ds(0x0010); /* data descriptor. */
    set_es(0x0010); /* extra data descriptor. */

    /* Return Back to Real Mode: */
    set_cr0(get_cr0() & 0x7FFFFFFE);

    /* Restore DS & ES: */
    set_ds(0x0000);
    set_es(0x0000);

    /* now it is safe to enable IRQs again. */
    sti();

}

void go_protected(uint32_t pc /* program counter */) {

    /* Don't forget to disable IRQs first */
    cli();

    /* Initialize GDTR structure: */
    gdtr.limit = sizeof(gdt)-1;
    gdtr.base  = (uint64_t) &gdt;

    /* Set LGDTR */
    lgdt(gdtr);

    /* Enter Protected Mode: */
    set_cr0(get_cr0() | 0x00000001);

    /* Update Code Selector Register (CS) */
    set_cs(0x0018);

    /* beginning of 32-bit code: */
    code32();

    /* Update Segment Selector Registers: */
    set_ds(0x0020);
    set_es(0x0020);
    set_fs(0x0020);
    set_gs(0x0020);
    set_ss(0x0020);

    /* Stack & base pointers: */
    set_esp(get_esp() & 0xFFFF);
    set_ebp(get_ebp() & 0xFFFF);

    /* Change PC: */
    call(pc);

    /* Interrupts might have been enabled. */
    cli();

    /* GDTR might have been changed. */
    lgdt(gdtr);

    /* Unreal mode IDTR structure: */
    idtr.limit = 0x03FF;
    idtr.base  = 0x00000000;

    /* Set IDTR Register: */
    lidt(idtr);

    /* Update Code Selector Register (CS) */
    set_cs(0x0008);

    /* beginning of 16-bit code: */
    code16();

    /* Update Segment Selector Registers: */
    set_ds(0x0010);
    set_es(0x0010);
    set_fs(0x0010);
    set_gs(0x0010);
    set_ss(0x0010);

    /* Back to unreal mode: */
    set_cr0(get_cr0() & 0x7FFFFFFE);

    /* Reset Code Selector Registers */
    set_cs(0x0000);
    set_ds(0x0000);
    set_es(0x0000);
    set_fs(0x0000);
    set_gs(0x0000);
    set_ss(0x0000);

    /* Enable IRQs: */
    sti();

}
