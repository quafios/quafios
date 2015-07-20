/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Synchronization testing.                         | |
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
#include <sys/printk.h>
#include <sys/scheduler.h>
#include <sys/proc.h>
#include <sys/mm.h>
#include <sys/fs.h>
#include <arch/stack.h>
#include <arch/spinlock.h>
#include <sys/semaphore.h>

int32_t in_synctest = 0;

int32_t busy = 0;
spinlock_t spinlock;
int32_t count = 1;

semaphore_t sema;

int32_t is_in_synctest() {
    return in_synctest;
}

void synctest_main() {
    int32_t i;
    while(1) {
        if (curproc->pid == 1) {
            /*spinlock_acquire(&spinlock);*/
            sema_down(&sema);
            printk("%aA", 0x4F);
            sema_up(&sema);
            /*spinlock_release(&spinlock);*/
        } else {
            /*spinlock_acquire(&spinlock);*/
            sema_down(&sema);
            printk("%aB", 0x1F);
            sema_up(&sema);
            /*spinlock_release(&spinlock);*/
        }
    }
}

void synctest() {
    in_synctest = 1;
    spinlock_init(&spinlock);
    sema_init(&sema, 1);
    fork();
    sema_down(&sema);
    yield();
    sema_up(&sema);
    synctest_main();
}
