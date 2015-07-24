/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Memory manager initialization.                   | |
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

#include <arch/type.h>
#include <sys/mm.h>
#include <lib/linkedlist.h>

char *mm_stats(int32_t *size) {
    extern linkedlist pfreelist;
    extern linkedlist freelist[33];
    extern int32_t pmem_usable_pages, kalloc_size, ram_size;
    char *buf = kmalloc(4096);
    int32_t i, sum = 0;
    /*buf = kmalloc(4096);*/
    buf[0] = 0;
    *size = 0;
    *size += sputs(&buf[*size], "Physical free:   ");
    *size += sputd(&buf[*size], pfreelist.count);
    *size += sputs(&buf[*size], " (");
    *size += sputd(&buf[*size], pfreelist.count*4);
    *size += sputs(&buf[*size], "KB)\n");
    *size += sputs(&buf[*size], "Physical usable: ");
    *size += sputd(&buf[*size], pmem_usable_pages);
    *size += sputs(&buf[*size], " (");
    *size += sputd(&buf[*size], pmem_usable_pages*4);
    *size += sputs(&buf[*size], "KB)\n");
    *size += sputs(&buf[*size], "Physical total:  ");
    *size += sputd(&buf[*size], ram_size);
    *size += sputs(&buf[*size], " (");
    *size += sputd(&buf[*size], ram_size*4);
    *size += sputs(&buf[*size], "KB)\n");
    *size += sputs(&buf[*size], "Buddy map: ");
    for (i = 0; i < 33; i++) {
        *size += sputd(&buf[*size], freelist[i].count);
        *size += sputs(&buf[*size], " ");
        sum += (1<<i)*freelist[i].count;
    }
    *size += sputs(&buf[*size], "\n            (");
    *size += sputd(&buf[*size], sum);
    *size += sputs(&buf[*size], ") ");
    *size += sputs(&buf[*size], "(");
    *size += sputd(&buf[*size], kalloc_size);
    *size += sputs(&buf[*size], ")");
    *size += sputs(&buf[*size], "\n");
    buf[*size] = 0;
    return buf;
}

void mm_init() {

    /* Initialize physical memory: */
    pmem_init();

    /* Initialize kernel memory: */
    kmem_init();

    /* register in sysfs */
    sysfs_reg("mem", mm_stats);

#if 0
    /* Output statistics: */
    printk("\n");
    printk("Physical Memory Manager Statistics: \n");
    printk("====================================\n");
    printk("kernel: start: %x, end: %x\n",
            KERNEL_PHYSICAL_START, KERNEL_PHYSICAL_END);
    printk("Accessible RAM Size (Approximate): %dMB.\n",
            ((pmem_usable_pages + 0x100 - 1) & 0xFFFFFF00)>>8);

    printk("\n");
    printk("Kernel Space Memory:\n");
    printk("=====================\n");
    printk("0x%x:0x%x - Kernel image.\n",
            KERNEL_VIRTUAL_START, KERNEL_VIRTUAL_END);
    printk("0x%x:0x%x - Lower memory image.\n",
            LOWMEM_VIRTUAL_START, LOWMEM_VIRTUAL_END);
#endif

}
