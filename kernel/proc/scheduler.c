/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> procman: task scheduler.                         | |
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
#include <sys/proc.h>
#include <sys/scheduler.h>

uint32_t scheduler_irq = 0xFFFFFFFF;
uint64_t ticks = 0;
uint8_t  scheduler_enabled = 0;
uint32_t flag = 0;

spinlock_t sched_lock = 0;

proc_t *curproc  = NULL;
proc_t *lastproc = NULL;

/* What is a task queue?
 * -----------------------
 * It is a queue of processes that are waiting for some event.
 * the queue structure "first" variable contains the PID of first
 * process in the queue. "last" contains the PID of last process.
 * in the process structure, process[x].next contains the PID
 * of next process in the queue that contains the process x.
 * process[x].prev contains the PID of the previous process
 * in the queue.
 */

pdlist_t q_ready = {0}, q_blocked;

/* "q_ready" is a linked list of tasks that are waiting to be executed.
 * Currently, no support for multi-processing. So, only CPU 0 is supported.
 * "q_blocked" is the queue of tasks that are waiting for some resource
 * to be free.
 */

void scheduler() {
    int32_t status;

    /* scheduler is enabled? */
    if (!scheduler_enabled)
        return;

    /* enter critical region */
    status = arch_get_int_status();
    arch_disable_interrupts();

    /* if task is blocked, release its lock_to_unlock if any */
    if (curproc->blocked && curproc->lock_to_unlock) {
        spinlock_release(curproc->lock_to_unlock);
        curproc->lock_to_unlock = NULL;
    }

    /* add current task to ready/waiting queue (if not blocked) */
    if (!curproc->blocked && !curproc->terminated)
        ENQUEUE(q_ready, curproc->sched);

    /* no ready processes? */
    if (!q_ready.count) {
        arch_set_int_status(status);
        return;
    }

    /* get the next task from ready/waiting queue */
    lastproc = curproc;
    curproc = ((pd_t *) DEQUEUE(q_ready))->proc;

    /* exit critical region */
    arch_set_int_status(status);

    /* switch processes:  */
    arch_proc_switch(lastproc, curproc);
}

void yield() {
    /* scheduler is enabled? */
    if (!scheduler_enabled)
        return;

    /* call arch-specific code */
    arch_yield();
}

int32_t getpid() {
    /* return PID of the caller */
    if (curproc)
        return curproc->pid;
    else
        return 0;
}

void unblock(int32_t pid) {
    int32_t status;

    /* get process structure by PID */
    proc_t *proc = get_proc(pid);

    /* enter critical region */
    status = arch_get_int_status();
    arch_disable_interrupts();

    /* is blocked? */
    if (proc && proc->blocked) {
        /* unblock */
        proc->blocked = 0;

        /* add to q_ready */
        if (curproc->pid != proc->pid)
            ENQUEUE(q_ready, proc->sched);
    }

    /* exit critical region */
    arch_set_int_status(status);
}

void block() {
    int32_t status;
    /* block current process */
    curproc->blocked = 1;
    /* call scheduler immediately */
    yield();
    /* all processes are blocked? */
    while (curproc->blocked) {
        status = arch_get_int_status();
        arch_enable_interrupts();
        yield();
        arch_set_int_status(status);
    }
}

void block_unlock(spinlock_t *spinlock) {
    int32_t status;
    /* block the task, then unlock some lock */
    curproc->lock_to_unlock = spinlock;
    curproc->blocked = 1;
    /* call scheduler immediately */
    yield();
    /* all processes are blocked? */
    while (curproc->blocked) {
        status = arch_get_int_status();
        arch_enable_interrupts();
        yield();
        arch_set_int_status(status);
    }
}

void sleep(uint64_t milliseconds) {
    uint64_t start = ticks;
    uint64_t end = ticks+milliseconds/10;
    while (ticks <= end)
        yield();
}
