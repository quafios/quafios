/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> Memory manager initialization.                   | |
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

void mm_init() {

    /* Initialize physical memory: */
    pmem_init();

    /* Initialize kernel memory: */
    kmem_init();

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
