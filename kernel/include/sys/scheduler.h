/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Task scheduler header.                           | |
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

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <arch/type.h>
#include <sys/proc.h>

#define SCHEDULER_INTERVAL_NS   10000000   /* 10ms */
#define SCHEDULER_INTERVAL      0.01       /* in seconds */

extern uint32_t scheduler_irq;
extern uint64_t ticks;
extern uint8_t  scheduler_enabled;
extern proc_t*  curproc;
extern pdlist_t q_ready, q_blocked;

void scheduler();
void sleep(uint64_t milliseconds);

#endif
