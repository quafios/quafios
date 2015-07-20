/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> memman: physical memory.                         | |
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

/* pmem.c
 * Physical Memory Management Unit.
 * I have no idea what i am doing here -_-'
 */

#include <arch/type.h>
#include <sys/error.h>
#include <lib/linkedlist.h>
#include <sys/mm.h>
#include <arch/page.h>
#include <sys/bootinfo.h>

#include <i386/asm.h> /* FIXME: arch-dependant stuff! */
#include <i386/stack.h> /* FIXME: arch-dependant stuff! */

uint32_t pmem_usable_pages = 0;
uint32_t ram_size = 0;

/* 4MB memory map: */
uint32_t pmmap[MEMORY_PAGES];

/* Free page list: */
linkedlist pfreelist;

/* boot info: */
extern bootinfo_t *bootinfo;

/* a kernel page for physical memory IO: */
uint8_t physical_page[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));
uint32_t cur_physical_page = NULL;

/* Access physical memory routine */

uint8_t pmem_readb(void *p_addr) {
    /* p_addr: physical memory address which is to be accessed. */

    /* p_addr = p_page (page base) + p_off (offset to that base). */
    uint32_t p_page = ((uint32_t) p_addr) & PAGE_BASE_MASK;
    uint32_t p_off  = ((uint32_t) p_addr) & (PAGE_SIZE-1);

    if (p_page != cur_physical_page)
        arch_set_page(NULL, physical_page, cur_physical_page=p_page);

    return physical_page[p_off];

}

uint16_t pmem_readw(void *p_addr) {
    return   (pmem_readb(((uint8_t *)p_addr)+0)<< 0)
           + (pmem_readb(((uint8_t *)p_addr)+1)<< 8);
}

uint32_t pmem_readl(void *p_addr) {
    return   (pmem_readb(((uint8_t *)p_addr)+0)<< 0)
           + (pmem_readb(((uint8_t *)p_addr)+1)<< 8)
           + (pmem_readb(((uint8_t *)p_addr)+2)<<16)
           + (pmem_readb(((uint8_t *)p_addr)+3)<<24);
}

void pmem_writeb(void *p_addr, uint8_t val) {

    /* p_addr: physical memory address which is to be accessed. */

    /* p_addr = p_page (page base) + p_off (offset to that base). */
    uint32_t p_page = ((uint32_t) p_addr) & PAGE_BASE_MASK;
    uint32_t p_off  = ((uint32_t) p_addr) & (PAGE_SIZE-1);

    if (p_page != cur_physical_page)
        arch_set_page(NULL, physical_page, cur_physical_page=p_page);

    physical_page[p_off] = val;

}

void pmem_writew(void *p_addr, uint16_t val) {
    pmem_writeb(((uint8_t *)p_addr)+0, (val>> 0) & 0xFF);
    pmem_writeb(((uint8_t *)p_addr)+1, (val>> 8) & 0xFF);
}

void pmem_writel(void *p_addr, uint32_t val) {
    pmem_writeb(((uint8_t *)p_addr)+0, (val>> 0) & 0xFF);
    pmem_writeb(((uint8_t *)p_addr)+1, (val>> 8) & 0xFF);
    pmem_writeb(((uint8_t *)p_addr)+2, (val>>16) & 0xFF);
    pmem_writeb(((uint8_t *)p_addr)+3, (val>>24) & 0xFF);
}

void *ppalloc() {

    linknode *entry;
    uint32_t eflags = get_eflags();
    cli();

    if (!pfreelist.count) {
        Regs regs = {0};
        panic(&regs, "Physical memory not enough!\n");
    }

    entry = pfreelist.first;
    linkedlist_remove(&pfreelist, pfreelist.first, NULL);

    set_eflags(eflags);

    return (void *)((((uint32_t) entry) - ((uint32_t) &pmmap))*
                    PAGE_SIZE/sizeof(uint32_t));

}


void ppfree(void *base) {

    linknode *entry;
    uint32_t eflags = get_eflags();
    cli();

    entry=(linknode*)((uint32_t)&pmmap[((uint32_t)base)/PAGE_SIZE]);
    linkedlist_add(&pfreelist, entry);

    set_eflags(eflags);
    return;

}

void pmem_init() {

    int32_t i;

    /* Initialize pfreelist: */
    linkedlist_init(&pfreelist);

    /* initialize the map itself: */
    for (i = 0; i < MEMORY_PAGES; i++)
        pmmap[i] = 0;

    /* update information about kernel area: */
    bootinfo->res[BI_KERNEL].end = KERNEL_PHYSICAL_END;

    /* read reserved regions: */
    for (i = 0; i < BI_RESCOUNT; i++) {
        /* read info: */
        uint64_t base = bootinfo->res[i].base;
        uint64_t end  = bootinfo->res[i].end;
        uint64_t frame;

        /* align to page boundaries.
         * example: 0x1234: 0x3456 ---> 0x1000:0x4000 (4KB page).
         */
        base = base & PAGE_BASE_MASK; /* align to lower. */
        end  = (end+PAGE_SIZE-1) & PAGE_BASE_MASK; /* to upper. */

        /* mark the page frames as reserved: */
        for (frame=base/PAGE_SIZE; frame<end/PAGE_SIZE; frame++) {
            if (frame < MEMORY_PAGES) {
                pmmap[frame] = 0xFFFFFFFF;
            }
        }
    }

    /* read RAM map: */
    for (i = 0; i < bootinfo->mem_ents; i++) {
        /* read info: */
        uint64_t base = bootinfo->mem_ent[i].base;
        uint64_t end  = bootinfo->mem_ent[i].end;
        uint64_t frame;

        /* align to page boundaries:
         * example: 0x1234: 0x3456 ---> 0x2000:0x3000 (4KB page).
         */
        base = (base+PAGE_SIZE-1) & PAGE_BASE_MASK; /* to upper. */
        end  = end & PAGE_BASE_MASK; /* align to lower. */

        /* mark the page frames as free: */
        for (frame=base/PAGE_SIZE; frame<end/PAGE_SIZE; frame++) {
            if (frame < MEMORY_PAGES &&
                pmmap[frame] != 0xFFFFFFFF) {
                /* a free page frame! */
                linkedlist_addlast(&pfreelist, (linknode *) &pmmap[frame]);
                pmem_usable_pages++;
            }
        }

    }

}
