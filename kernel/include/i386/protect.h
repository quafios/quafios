/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> i386: protected mode header.                     | |
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

#ifndef PROTECT_H
#define PROTECT_H

#include <arch/type.h>

/* Privilege Levels:  */
/* ------------------ */
#define PL_KERNEL       0
#define PL_USER         3

/* CR0:  */
/* ----- */
#define CR0_PE          0x00000001  /* Protected Mode Enable. */
#define CR0_MP          0x00000002  /* Monitor co-processor.  */
#define CR0_EM          0x00000004  /* Emulation.             */
#define CR0_TS          0x00000008  /* Task Switched.         */
#define CR0_ET          0x00000010  /* Extension Type.        */
#define CR0_NE          0x00000020  /* Numeric Error.         */
#define CR0_WP          0x00010000  /* Write Protect.         */
#define CR0_AM          0x00040000  /* Alignment Mask.        */
#define CR0_NW          0x20000000  /* Not-write through.     */
#define CR0_CD          0x40000000  /* Cache disable.         */
#define CR0_PG          0x80000000  /* Paging.                */

#define CR0_GENERIC     (CR0_PE /* | CR0_NW | CR0_CD */ | CR0_PG)

/* GDT:  */
/* ----- */
/* Global Descriptor Table */
#define GDT_ENTRY_FREE          0
#define GDT_ENTRY_TS            1
#define GDT_ENTRY_KERNEL_CODE   2
#define GDT_ENTRY_KERNEL_DATA   3
#define GDT_ENTRY_USER_CODE     4
#define GDT_ENTRY_USER_DATA     5

#define GDT_SEGMENT_SELECTOR(entry) (entry<<3)

#define GDT_ENTRY_COUNT         6

/* Segment Descriptor EType Field:
 * Bit 0: Accessed, for code and data selectors.
 * Bit 1: Readable Bit for Code Selectors.
 *        Writable Bit for Data Selectors.
 * Bit 2: Conforming Bit for Code Selectors
 *        [0: Code be executed only from the ring set in DPL,
 *         1: Code can be executed from an equal or lower PL by a far jump].
 *        Direction Bit for Data Selectors [0: grows up, 1: grows down].
 * Bit 3: Executable Bit [0: Data Selector, 1: Code Selector].
 * Bit 4: Should be 0 for special system segments.
 *        Should be 1 for applications code and data segments.
 */

#define GDT_ETYPE_CODE          0x1A
#define GDT_ETYPE_DATA          0x12
#define GDT_ETYPE_TS            0x09

#define GDT_PRESENT             1
#define GDT_NOT_PRESENT         0

/* Segment Descriptor Flags Field:
 * Bit 0: Available for the system to use it the way it likes.
 * Bit 1: Should be zero
 * Bit 2: Mode. [0: 16-bit pmode, 1: 32-bit pmode].
 * Bit 3: Granularity of "Limit" field. [0: use 1B blocks, 1: use 4KB blocks].
 */

#define GDT_Flags_DEFAULT       0xC
#define GDT_Flags_TS            0x4

#define GDT_LIMIT_LO_DEFAULT    0xFFFF
#define GDT_LIMIT_HI_DEFAULT    0xF

#define GDT_BASE_LO_DEFAULT     0x000000
#define GDT_BASE_HI_DEFAULT     0x00

struct segment_descriptor {
    unsigned limit_lo :16; /* Lower 16 bits of "Limit" field. */
    unsigned base_lo  :24; /* Lower 24 bits of "Base" field.  */
    unsigned etype    :5;  /* Type of segment.                */
    unsigned dpl      :2;  /* Descriptor privilege level.     */
    unsigned present  :1;  /* Segment present?                */
    unsigned limit_hi :4;  /* Higher 8 bits of "Limit" field. */
    unsigned flags    :4;  /* Flags.                          */
    unsigned base_hi  :8;  /* Higher 8 bits of "Base" field.  */
} __attribute__ ((packed));

extern struct segment_descriptor gdt[];

/* IDT:  */
/* ----- */
/* Interrupt Descriptor Table */

#define IDT_ENTRY_COUNT         256

#define IDT_ETYPE_TASK          0x0500 /* Task Gate.      */
#define IDT_ETYPE_INTERRUPT     0x0E00 /* Interrupt Gate. */
#define IDT_ETYPE_TRAP          0x0F00 /* Trap Gate.      */

#define IDT_PRESENT             1
#define IDT_NOT_PRESENT         0

struct gate_descriptor {
    unsigned offset_lo :16;
    unsigned selector  :16;
    unsigned etype     :13;
    unsigned dpl       :2;
    unsigned present   :1;
    unsigned offset_hi :16;
} __attribute__ ((packed));

extern struct gate_descriptor idt[];

/* Task Segment:  */
/* -------------- */

struct task_struct {
    uint32_t back_link;
    uint32_t esp0, ss0;
    uint32_t esp1, ss1;
    uint32_t esp2, ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax,ecx,edx,ebx;
    uint32_t esp, ebp;
    uint32_t esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint32_t trace_bitmap;
} __attribute__ ((packed));

extern struct task_struct ts;

/* IRQs: */
#define IRQ_COUNT               0x10

#endif
