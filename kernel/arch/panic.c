/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> i386: panic() procedure.                         | |
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
#include <sys/scheduler.h>
#include <sys/printk.h>
#include <tty/vtty.h>

#include <i386/asm.h>
#include <i386/protect.h>
#include <i386/stack.h>
#include <i386/page.h>

void panic(Regs *regs, const char *fmt, ...) {

    uint32_t ebp;
    uint32_t cr0, cr2, cr3, esp;
    uint16_t ss;
    int32_t pid;
    extern uint8_t bg_attrib;

    cli();

    /* enter text mode */
    legacy_video_clear(bg_attrib = 0x1F);
    vtty_ioctl(system_console, TTY_ENABLE, NULL);

    printk("\n Kernel Panic!\n ===============\n ");

    __asm__("mov %%ebp, %%eax":"=a"(ebp));
    printk_byaddr((uint32_t *) (ebp + 12));

    /* Print register state: */
    __asm__("movl    %%cr0,        %%eax":"=a"(cr0 ));
    __asm__("movl    %%cr2,        %%eax":"=a"(cr2 ));
    __asm__("movl    %%cr3,        %%eax":"=a"(cr3 ));

    if (regs->cs != GDT_SEGMENT_SELECTOR(GDT_ENTRY_KERNEL_CODE)) {
        /* PL change */
        esp = regs->umode_esp;
        ss  = regs->umode_ss;
    } else {
        esp = regs->esp + 16;
        ss  = GDT_SEGMENT_SELECTOR(GDT_ENTRY_KERNEL_DATA);
    }

    if (curproc) {
        pid = curproc->pid;
    } else {
        pid = 0;
    }

    printk("\n");
    printk("\n CS : 0x%x     EIP: 0x%x     EF : 0x%x     ERR: 0x%x",
           regs->cs, regs->eip, regs->eflags, regs->err);
    printk("\n SS : 0x%x     ESP: 0x%x     PID: 0x%x              ",
           ss, esp, pid);
    printk("\n");
    printk("\n CONTROL REGISTERS: ");
    printk("\n -----------------------");
    printk("\n CR0: 0x%x     CR2: 0x%x     CR3: 0x%x              ",
           cr0, cr2, cr3);
    printk("\n");
    printk("\n GENERAL REGISTERS:");
    printk("\n -----------------------");
    printk("\n EAX: 0x%x     EBX: 0x%x     ECX: 0x%x     EDX: 0x%x",
           regs->eax, regs->ebx, regs->ecx, regs->edx);
    printk("\n ESI: 0x%x     EDI: 0x%x     EBP: 0x%x              ",
           regs->esi, regs->edi, regs->ebp);
    printk("\n");
    printk("\n SEGMENT SELECTORS:");
    printk("\n -----------------------");
    printk("\n DS : 0x%x     ES : 0x%x     FS : 0x%x     GS : 0x%x",
           regs->ds, regs->es, regs->fs, regs->gs);

    __asm__("jmp ."::"a"(regs->eip), "b"(curproc->pid));

    idle();
}
