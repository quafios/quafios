/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> memman: kernel memory.                           | |
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
#include <lib/linkedlist.h>
#include <sys/error.h>
#include <sys/mm.h>
#include <arch/page.h>

#define L       4   /* 2^L is the smallest size block (16 Bytes) */
#define U       30  /* 2^U is the upper size block (1GB)         */

linkedlist freelist[32], usedlist;

/* Bianry Operations:  */
/* ------------------- */
#define pow2(n)         (((uint32_t) 1<<n))
#define nextpow2(n)     (__extension__({            \
                             uint32_t num = n;  \
                            --num;                  \
                            num |= num >> 1;        \
                            num |= num >> 2;        \
                            num |= num >> 4;        \
                            num |= num >> 8;        \
                            num |= num >> 16;       \
                            ++num;                  \
                            num;                    \
                        }))

/* Reference:
 * http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn
 */
static const int MultiplyDeBruijnBitPosition2[32] = {
  0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
  31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};
#define log2(n) (MultiplyDeBruijnBitPosition2[\
                    (uint32_t)(n * 0x077CB531U) >> 27])

int32_t kmem_initialized = 0;

/* ======================================================================== */
/*                          Allocation Functions                            */
/* ======================================================================== */

uint32_t get_hole(uint32_t i) {
    if (i > U) return NULL;
    if (i < L) i = L;
    if (freelist[i].count) {
        linknode *ptr = freelist[i] .first;
        /* Remove first element from the free list. */
        linkedlist_remove(&freelist[i], ptr, NULL);
        if (i > 11) arch_vmpage_unmap(NULL, ptr);
        return (uint32_t) ptr;
    } else {
        uint32_t ret = get_hole(i+1);
        if (ret == NULL)
            return ret;
        /* Split ret to two (2^i) chunks, one is free, the other returned. */
        arch_vmpage_map(NULL, ret + pow2(i), 0);
        linkedlist_add(&freelist[i], (linknode *) (ret + pow2(i)));
        /* printk("ret: %x %dBytes\n", ret, pow2(i)); */
        return ret;
    }
}

void *kmalloc(uint32_t size) {
    /* local vars */
    linknode *ptr;
    uint32_t base, i;

    /* Get free hole: */
    base = get_hole(log2(nextpow2(size)));

    /* Allocate physical pages: */
    for (i = base & 0xFFFFF000; i < base+size; i+=PAGE_SIZE)
        arch_vmpage_map(NULL, i, 0);

    /* Store info about this allocated space... */
    ptr = (linknode *) get_hole(log2(16));
    arch_vmpage_map(NULL, (uint32_t) ptr, 0);
    linkedlist_add(&usedlist, ptr);
    ptr->datum[0] = base;
    ptr->datum[1] = size;
    return (void *) base;
}

void kfree(void *ptr) {

}

/* ======================================================================== */
/*                              Initialization                              */
/* ======================================================================== */

void kmem_init() {

    /* initialize free lists:  */
    /* ----------------------- */
    uint32_t i;
    for (i = 0; i < 32; i++)
        linkedlist_init(&freelist[i]);
    linkedlist_init(&usedlist);

    arch_vmpage_map(NULL, KERNEL_MEMORY_BASE, 0);
    linkedlist_add(&freelist[U], (linknode *)((uint32_t) KERNEL_MEMORY_BASE));

    kmem_initialized = 1;

}
