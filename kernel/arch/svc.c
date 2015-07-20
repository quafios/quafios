/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> i386 - ISR - system call handler.                | |
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

/* System Call Handler is actually the interface between running applications
 * and kernel. Applications do invoke system calls via interrupt gate 0x80.
 */

#include <arch/type.h>
#include <sys/error.h>
#include <sys/syscall.h>
#include <sys/scheduler.h>

#include <i386/asm.h>
#include <i386/protect.h>
#include <i386/stack.h>
#include <i386/page.h>

void svc () {

    /* a system call signal recieved...
     * please refer to include/syscall.h...
     */

    Regs *regs;
    __asm__("lea 8(%%ebp), %%eax":"=a"(regs));

    set_eflags(regs->eflags);

    curproc->context = (void *) regs;

    regs->eax = syscall(regs->eax, regs->ebx, regs->ecx,
                    regs->edx, regs->esi, regs->edi);

    cli(); /* important! */

}

void syscall_gate() {
    __asm__("SYSTEM_CALL:");
        __asm__("pushl $0");
        store_reg();
        svc();
        restore_reg();
        __asm__("add $4, %esp");
        iret();
}

void syscall_init() {
    /* Initialize IDT System Call Gate: */
    uint32_t offset;
    __asm__("movl $SYSTEM_CALL, %%eax":"=a"(offset));

    idt[0x80].offset_lo = offset&0xFFFF;
    idt[0x80].selector = GDT_SEGMENT_SELECTOR(GDT_ENTRY_KERNEL_CODE);
    idt[0x80].etype = IDT_ETYPE_INTERRUPT;
    idt[0x80].dpl = PL_USER;
    idt[0x80].present = IDT_PRESENT;
    idt[0x80].offset_hi = offset>>16;
}
