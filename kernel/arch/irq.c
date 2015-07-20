/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> i386: ISR - IRQ handler.                         | |
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
#include <arch/irq.h>
#include <sys/error.h>
#include <sys/mm.h>
#include <sys/device.h>
#include <sys/scheduler.h>
#include <sys/semaphore.h>
#include <pic/8259A.h>

#include <i386/asm.h>
#include <i386/protect.h>
#include <i386/stack.h>
#include <i386/page.h>
#include <i386/spinlock.h>

#define irq(irq_num)                                    \
            __asm__("pushl $0");                        \
            store_reg();                                \
            __asm__("sti");                             \
            __asm__("push %%eax"::"a"(irq_num));        \
            __asm__("call *%%eax"::"a"(irq_handler));   \
            __asm__("popl %eax");                       \
            restore_reg();                              \
            __asm__("add $4, %esp");                    \
            iret();

#define IRQ_COUNT 0x10

typedef struct {
    uint32_t    usable;      /* can be used?              */
    device_t   *pic_device;  /* Reservation Queue.        */
    linkedlist  requeue;     /* reservation queue (FCFS). */
} irq_t;

irq_t irq[IRQ_COUNT] = {0};

semaphore_t irqsema;

int32_t irq_setup(uint32_t n, device_t *pic_device) {

    if (n >= IRQ_COUNT)
        return ENOENT;

    /* setup an IRQ entry: */
    sema_down(&irqsema);
    irq[n].pic_device = pic_device;
    irq[n].usable     = 1;
    sema_up(&irqsema);

    /* return: */
    return ESUCCESS;
}

uint32_t irq_reserve(uint32_t n, irq_reserve_t *reserve) {

    int32_t status;

    if (n >= IRQ_COUNT || !irq[n].usable)
        return ENOENT;

    /* enter critical region */
    status = arch_get_int_status();
    arch_disable_interrupts();
    sema_down(&irqsema);

    /* add the request to the tail of the queue: */
    linkedlist_addlast(&(irq[n].requeue), reserve);

    /* exit critical region */
    sema_up(&irqsema);
    arch_set_int_status(status);

    /* done. */
    return ESUCCESS;

}

void irq_handler(uint32_t n) {

    int32_t status;

    if (n >= IRQ_COUNT || !irq[n].usable)
        return; /* nothing to do here. */

    /* enter critical region */
    status = arch_get_int_status();
    arch_disable_interrupts();
    sema_down(&irqsema);

    if (irq[n].requeue.count) {
        /* The IRQ is to be served (apply FIRST COME FIRST SERVED). */
        irq_reserve_t *req     = (irq_reserve_t *) irq[n].requeue.first;
        device_t      *dev     = req->dev;
        uint32_t       expires = req->expires;
        void          *data    = req->data;
        if (expires) {
            /* delete the request from the queue */
            linkedlist_aremove(&(irq[n].requeue), (linknode *) req);
        }
        /* now serve it! */
        dev_irq(dev, n, data);
    }

    /* exit critical region */
    sema_up(&irqsema);
    arch_set_int_status(status);

    /* Send End of Interrupt Command: */
    dev_ioctl(irq[n].pic_device, PIC_CMD_EOI, NULL);

    /* call scheduler? */
    if (n == scheduler_irq) {
        ticks++;
        scheduler();
    }

}

int32_t irq_to_vector(uint32_t n) {
    return n + 0x20;
}

void enable_irq_system() {
    sti();
}

void irq_gate() {

    __asm__("IRQ00:"); irq(0x00);
    __asm__("IRQ01:"); irq(0x01);
    __asm__("IRQ02:"); irq(0x02);
    __asm__("IRQ03:"); irq(0x03);
    __asm__("IRQ04:"); irq(0x04);
    __asm__("IRQ05:"); irq(0x05);
    __asm__("IRQ06:"); irq(0x06);
    __asm__("IRQ07:"); irq(0x07);
    __asm__("IRQ08:"); irq(0x08);
    __asm__("IRQ09:"); irq(0x09);
    __asm__("IRQ0A:"); irq(0x0A);
    __asm__("IRQ0B:"); irq(0x0B);
    __asm__("IRQ0C:"); irq(0x0C);
    __asm__("IRQ0D:"); irq(0x0D);
    __asm__("IRQ0E:"); irq(0x0E);
    __asm__("IRQ0F:"); irq(0x0F);

}

void irq_init() {

    /* Initialize IDT IRQ Descriptors: */
    uint32_t offset[0x10], i;

    __asm__("movl $IRQ00, %%eax":"=a"(offset[0x0]));
    __asm__("movl $IRQ01, %%eax":"=a"(offset[0x1]));
    __asm__("movl $IRQ02, %%eax":"=a"(offset[0x2]));
    __asm__("movl $IRQ03, %%eax":"=a"(offset[0x3]));
    __asm__("movl $IRQ04, %%eax":"=a"(offset[0x4]));
    __asm__("movl $IRQ05, %%eax":"=a"(offset[0x5]));
    __asm__("movl $IRQ06, %%eax":"=a"(offset[0x6]));
    __asm__("movl $IRQ07, %%eax":"=a"(offset[0x7]));
    __asm__("movl $IRQ08, %%eax":"=a"(offset[0x8]));
    __asm__("movl $IRQ09, %%eax":"=a"(offset[0x9]));
    __asm__("movl $IRQ0A, %%eax":"=a"(offset[0xA]));
    __asm__("movl $IRQ0B, %%eax":"=a"(offset[0xB]));
    __asm__("movl $IRQ0C, %%eax":"=a"(offset[0xC]));
    __asm__("movl $IRQ0D, %%eax":"=a"(offset[0xD]));
    __asm__("movl $IRQ0E, %%eax":"=a"(offset[0xE]));
    __asm__("movl $IRQ0F, %%eax":"=a"(offset[0xF]));

    for (i = 0x00; i < 0x10; i++) {
        idt[i+0x20].offset_lo = offset[i]&0xFFFF;
        idt[i+0x20].selector  = GDT_SEGMENT_SELECTOR(GDT_ENTRY_KERNEL_CODE);
        idt[i+0x20].etype     = IDT_ETYPE_INTERRUPT;
        idt[i+0x20].dpl       = PL_KERNEL;
        idt[i+0x20].present   = IDT_PRESENT;
        idt[i+0x20].offset_hi = offset[i]>>16;
        linkedlist_init(&(irq[i].requeue));
    }

    sema_init(&irqsema, 1);

}
