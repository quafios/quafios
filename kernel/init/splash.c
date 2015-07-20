/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Splash Screen.                                   | |
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

void splash() {

    uint32_t i;

    printk("\n");

    printk("%a", 0x0B);

    for (i = 0; i < 80; i++)
            printk("=");
    printk("                                 Quafios 2.0.1\n");
    for (i = 0; i < 80; i++)
            printk("=");

    printk("\n");

    printk("%a", 0x0A);
    printk("Quafios Copyright (C) 2015 Mostafa Abd El-Aziz.\n");
    printk("This program comes with ABSOLUTELY NO WARRANTY; ");
    printk("for details\ntype `view /home/DISCLAIMER'.\n");
    printk("This is free software, ");
    printk("and you are welcome to redistribute it under certain\n");
    printk("conditions; type `view /home/COPYING' ");
    printk("for details.\n");
    printk("%a", 0x0F);

}
