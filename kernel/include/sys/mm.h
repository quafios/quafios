/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Memory management header.                        | |
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

#ifndef MEMMAN_H
#define MEMMAN_H

#include <lib/linkedlist.h>
#include <arch/type.h>

#define NULL            0

/* IF 32 BIT: */
#define MEMORY_LIMIT        __extension__ 0x100000000ULL    /* 4GB. */
#define MEMORY_PAGES        ((uint32_t)(MEMORY_LIMIT/PAGE_SIZE))

extern uint32_t kernel_physical_start;
extern uint32_t kernel_physical_end;

#define KERNEL_PHYSICAL_START   ((uint32_t) &kernel_physical_start)

#define KERNEL_PHYSICAL_END     ((uint32_t) &kernel_physical_end)

#define KERNEL_SIZE             (KERNEL_PHYSICAL_END-KERNEL_PHYSICAL_START)

#define KTEXT_MEMORY_BASE       0x00000000
#define KTEXT_MEMORY_END        0x08000000
#define KTEXT_MEMORY_SIZE       (KTEXT_MEMORY_END-KTEXT_MEMORY_BASE)
#define KTEXT_MEMORY_PAGES      (KTEXT_MEMORY_SIZE/PAGE_SIZE)
#define KTEXT_MEMORY_PTABLES    (KTEXT_MEMORY_PAGES/PAGE_TABLE_ENTRY_COUNT)

#define USER_MEMORY_BASE        KTEXT_MEMORY_END
#define USER_MEMORY_END         0xC0000000
#define USER_MEMORY_SIZE        (USER_MEMORY_END-USER_MEMORY_BASE)
#define USER_MEMORY_PAGES       (USER_MEMORY_SIZE/PAGE_SIZE)
#define USER_MEMORY_PTABLES     (USER_MEMORY_PAGES/PAGE_TABLE_ENTRY_COUNT)

#define KERNEL_MEMORY_BASE      USER_MEMORY_END
#define KERNEL_MEMORY_END       MEMORY_LIMIT
#define KERNEL_MEMORY_SIZE      ((uint32_t)(\
                                    KERNEL_MEMORY_END-KERNEL_MEMORY_BASE))
#define KERNEL_MEMORY_PAGES     (KERNEL_MEMORY_SIZE/PAGE_SIZE)
#define KERNEL_MEMORY_PTABLES   (KERNEL_MEMORY_PAGES/PAGE_TABLE_ENTRY_COUNT)

/* User Memory:  */
/* ============= */

typedef struct ummap_entry_str {
    struct ummap_entry_str *next;
    uint32_t base; /* base linear address */
    uint32_t size; /* size of block. */

    #define MMAP_TYPE_ANONYMOUS 0
    #define MMAP_TYPE_FILE      1
    #define MMAP_TYPE_STACK     2
    #define MMAP_TYPE_HEAP      3
    uint32_t type; /* FILE, ANONYMOUS, HEAP, STACK. */

    #define MMAP_FLAGS_READ     1
    #define MMAP_FLAGS_WRITE    2
    #define MMAP_FLAGS_EXEC     4
    #define MMAP_FLAGS_SHARED   8
    uint32_t flags;

    int32_t fd; /* file descriptor <if it is file>. */
    uint64_t foffset; /* offset in the file. */
} ummap_entry;

/* mapped file mem region */
typedef struct file_mem {
    struct file_mem *next;
    struct file *file; /* the file that corresponds to this region */
    uint64_t pos;      /* position of the mapping (offset in the file) */
    uint32_t paddr;    /* phyiscal memory address */
    uint32_t ref;      /* how many people use this? */
} file_mem_t;

/* Process memory Image: */
typedef struct {
    /* heap parameters: */
    uint32_t heap_start;
    uint32_t brk_addr;
    uint32_t heap_end;

    /* arch dependant stuff: */
    void *arch_reg;
} umem_t;

/* mmap arguments */
typedef struct {
    void *base;
    unsigned int size;
    unsigned int type;
    unsigned int flags;
    int fd;
    uint64_t off;
} mmap_arg_t;

/* prototype: */
void *kmalloc(uint32_t);

#endif
