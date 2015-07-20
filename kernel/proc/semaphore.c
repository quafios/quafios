/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> procman: semaphores.                             | |
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
#include <arch/spinlock.h>
#include <sys/mm.h>
#include <sys/semaphore.h>
#include <sys/scheduler.h>

void sema_init(semaphore_t *sema, int32_t counter) {
    spinlock_init(&sema->spinlock);
    sema->counter = counter;
    sema->head = NULL;
    sema->tail = NULL;
}

void sema_down(semaphore_t *sema) {
    int32_t status;
    /* enter critical region */
    status = arch_get_int_status();
    arch_disable_interrupts();
    spinlock_acquire(&sema->spinlock);
    /* decrease semaphore counter */
    sema->counter--;
    /* special case: scheduler not initialized yet */
    if (!scheduler_enabled) {
        spinlock_release(&sema->spinlock);
        arch_set_int_status(status);
        while (sema->counter < 0);
    }
    /* block? */
    if (sema->counter < 0) {
        /* add this process to the queue */
        curproc->semad.next = NULL;
        if (sema->head) {
            sema->tail = sema->tail->next = &curproc->semad;
        } else {
            sema->head = sema->tail = &curproc->semad;
        }
        /* exit critical region with a block */
        block_unlock(&sema->spinlock);
        arch_set_int_status(status);
    } else {
        /* exit critical region normally */
        spinlock_release(&sema->spinlock);
        arch_set_int_status(status);
    }
}

void sema_up(semaphore_t *sema) {
    int32_t status;
    /* enter critical region */
    status = arch_get_int_status();
    arch_disable_interrupts();
    spinlock_acquire(&sema->spinlock);
    /* increase counter */
    sema->counter++;
    /* queue is not empty? */
    if (sema->counter <= 0 && sema->head) {
        proc_t *proc = sema->head->proc;
        sema->head = sema->head->next;
        if (!sema->head)
            sema->tail = NULL;
        unblock(proc->pid);
    }
    /* exit critical region normally */
    spinlock_release(&sema->spinlock);
    arch_set_int_status(status);
}
