/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> procman: task scheduler.                         | |
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

#include <arch/type.h>
#include <sys/proc.h>
#include <sys/scheduler.h>

uint32_t scheduler_irq = 0xFFFFFFFF;
uint64_t ticks = 0;
uint8_t  scheduler_enabled = 0;
uint32_t flag = 0;

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

    ticks++;
    if (!scheduler_enabled) return;

    /* 1- Add current task to ready/waiting queue:  */
    /* -------------------------------------------- */
    if (!curproc->blocked && !curproc->terminated)
        ENQUEUE(q_ready, curproc->sched);
    if (!q_ready.count) {
        return;
    }

    /* 2- Get the next task from ready/waiting queue:  */
    /* ----------------------------------------------- */
    lastproc = curproc;
    curproc = ((pd_t *) DEQUEUE(q_ready))->proc;

    /* 3- Switch processes:  */
    /* --------------------- */
    arch_proc_switch(lastproc, curproc);

}

int32_t getpid() {

    return curproc->pid;

}

void unblock(int32_t pid) {

    proc_t *proc = get_proc(pid);

    /* isn't blocked? */
    if (!proc || !proc->blocked)
        return;

    /* unblock */
    proc->blocked = 0;

    /* add to q_ready */
    if (curproc->pid != proc->pid)
        ENQUEUE(q_ready, proc->sched);

}

void block() {

    /* block current process */
    curproc->blocked = 1;

    /* loop until the scheduler removes this process */
    while(curproc->blocked);

#if 0
    /* loop if I am the only process in the list */
    while (curproc->blocked && q_ready.count == 0);

    /* I got unblocked */
    if (!curproc->blocked)
        return;

    /* get next process in the queue */
    lastproc = curproc;
    curproc = ((pd_t *) DEQUEUE(q_ready))->proc;

    /* activate the next process */
    arch_proc_switch(lastproc, curproc);
#endif
}

void sleep(uint64_t milliseconds) {
    uint64_t start = ticks;
    uint64_t end = ticks+milliseconds/10;
    while(ticks <= end);
}
