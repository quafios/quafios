/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Intel 8259A chip header .                        | |
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
#include <sys/device.h>

/* Initialization Data: */
typedef struct {
    uint32_t baseirq;     /* The base irq of the range associated to PIC. */
#define CASCADE_NOPE    0
#define CASCADE_MASTER  1 /* in cascade mode, i am master.                */
#define CASCADE_SLAVE   2 /* in cascade mode, i am slave.                 */
    uint32_t cascade;
    uint32_t casirq;      /* irq of the co-PIC (only in cascade mode).    */
    device_t *master;     /* device structure of the master PIC           */
    uint8_t  irqenable;   /* 0: nope, 1: enable IRQs after init.          */
} i8259_init_t;

/* PIC Commands: */
#define PIC_CMD_EOI     0x20
