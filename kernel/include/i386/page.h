/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> i386: page header.                               | |
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

#ifndef PAGE_H
#define PAGE_H

#include <arch/type.h>

/* Paging:  */
/* ======== */
#define PAGE_SIZE               4096
#define PAGE_BASE_MASK          0xFFFFF000
#define PAGE_FLAG_MASK          0x00000FFF
#define PAGE_DIR_ENTRY_COUNT    1024
#define PAGE_TABLE_ENTRY_COUNT  1024
#define PAGE_ENTRY_SIZE         4
#define PAGE_DIR_SIZE           (PAGE_SIZE*PAGE_DIR_ENTRY_COUNT)

/* Page Entry:  */
/* ------------ */
struct page_entry {
    unsigned present     :  1;
    unsigned rw          :  1; /* Read/Write */
    unsigned us          :  1; /* User/Supervisor */
    unsigned reserved1   :  2;
    unsigned access      :  1;
    unsigned dirty       :  1;
    unsigned reserved2   :  2;
    unsigned to_be_alloc :  1; /* to be allocated */
    unsigned available   :  2;
    unsigned page_frame  : 20; /* Page Frame ID. */
} __attribute__ ((packed));

#define PAGE_ENTRY_P    0x001
#define PAGE_ENTRY_RW   0x002
#define PAGE_ENTRY_US   0x004
#define PAGE_ENTRY_AF   0x200 /* Allocated Flag */

#define PAGE_ENTRY_KERNEL_MODE  (PAGE_ENTRY_P | PAGE_ENTRY_RW)
#define PAGE_ENTRY_USER_MODE    (PAGE_ENTRY_P | PAGE_ENTRY_RW | PAGE_ENTRY_US)

#define PAGE_EXT_REMOVABLE      0x800

/* Page Table:  */
/* ------------ */
struct page_table {
    uint32_t entry[PAGE_TABLE_ENTRY_COUNT];
    /* 1024 page entries for each Page Table. */
} __attribute__ ((packed));

/* Common Page Tables:  */
/* ==================== */
extern uint32_t general_pagedir[];
extern uint32_t ktext_pagetab[];
extern uint32_t kmem_pagetab[];

#endif
