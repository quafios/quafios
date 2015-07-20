/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> procman: interprocess communication.             | |
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
#include <sys/error.h>
#include <sys/proc.h>
#include <sys/mm.h>
#include <sys/ipc.h>
#include <sys/scheduler.h>

int32_t send(int32_t pid, msg_t *msg) {

    msg_t *kmsg;
    proc_t *recp;
    int i;

    /* find the proc structure of the receiver */
    if (!(recp = (proc_t *) get_proc(pid)))
        return -EINVAL;

    /* allocate kernel structure for the message */
    kmsg = kmalloc(sizeof(msg_t));
    if (!kmsg)
        return -ENOMEM;

    /* allocate kernel buffer */
    kmsg->buf = kmalloc(msg->size);
    if (!(kmsg->buf)) {
        kfree(kmsg);
        return -ENOMEM;
    }

    /* initialize message structure */
    kmsg->sender = curproc->pid;
    kmsg->size   = msg->size;

    /* copy message */
    for (i = 0; i < msg->size; i++)
        ((uint8_t *) kmsg->buf)[i] = ((uint8_t *) msg->buf)[i];

    /* lock receiver's inbox */
    while (recp->inbox_lock);
    recp->inbox_lock = 1;

    /* add the message to the inbox of the receiver */
    linkedlist_addlast(&(recp->inbox), kmsg);

    /* unblock */
    if (recp->blocked_for_msg) {
        recp->blocked_for_msg = 0;
        unblock(recp->pid);
    }

    /* unlock the inbox */
    recp->inbox_lock = 0;

    /* success */
    return ESUCCESS;

}

int32_t receive(msg_t *msg, int wait) {

    msg_t *kmsg;
    int32_t i;

    /* inbox is empty? */
    if (!(curproc->inbox.count)) {
        if (!wait)
            return -ENOENT;
        while (!(curproc->inbox.count)) {
            curproc->blocked_for_msg = 1;
            block();
        }
    }

    /* unlock the inbox */
    while (curproc->inbox_lock);
    curproc->inbox_lock = 1;

    /* fetch the oldest message */
    kmsg = curproc->inbox.first;

    /* delete it from inbox */
    linkedlist_aremove(&(curproc->inbox), kmsg);

    /* unlock inbox */
    curproc->inbox_lock = 0;

    /* copy the message to user space */
    msg->size   = kmsg->size;
    msg->sender = kmsg->sender;
    for (i = 0; i < kmsg->size; i++)
        ((uint8_t *) msg->buf)[i] = ((uint8_t *) kmsg->buf)[i];

    /* deallocate kernel structures */
    kfree(kmsg->buf);
    kfree(kmsg);

    /* done */
    return ESUCCESS;

}
