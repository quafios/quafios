/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> i386: spinlocks.                                 | |
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

#include <i386/spinlock.h>
#include <i386/type.h>
#include <sys/mm.h>

void spinlock_init(spinlock_t *spinlock) {
    /* initialize spinlock to 0 (currently not acquired) */
    *spinlock = 0;
}

void spinlock_acquire(spinlock_t *spinlock) {
    /* x86 bts (bit test and set) instruction just works as follows:
     * load the current value of the targeted bit into CF flag (test)
     * set the value of the targeted bit to 1 (set).
     * LOCK prefix guarantees that this operation is atomic and no
     * memory access by any other CPU happens between the set
     * and test operation (it actually locks the bus).
     *
     * You may refer to the OS bible (2.2.3 - mutual exclusion) for
     * information about test and set lock (TSL) instructions. BTS
     * is also equivalent in some sense to the compare_and_swap()
     * instruction described in Stallings (5.2, 7th edition) with
     * *word = *spinlock, testval = 0, and newval = 1.
     */
    __asm__("1: lock bts $0, (%%esi);\n"
            "   jc       1b         ;\n"::"S"(spinlock));
}


void spinlock_release(spinlock_t *spinlock) {
    /* set spinlock to zero to let other processes
     * be able to hold it by spinlock_acquire().
     */
    *spinlock = 0;
}
