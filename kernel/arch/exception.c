/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> i386: ISR - exception handler.                   | |
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

#include <i386/asm.h>
#include <i386/protect.h>
#include <i386/stack.h>

#define EXP_NO_ERR           0
#define EXP_WITH_ERR         1
#define exp(exp_num, exp_err)                           \
            if (exp_err == EXP_NO_ERR)                  \
                __asm__("pushl $0");                    \
            store_reg();                                \
            __asm__("pushl %%eax"::"a"(exp_num));       \
            __asm__("call *%%eax"::"a"(exception));     \
            __asm__("popl %eax");                       \
            restore_reg();                              \
            __asm__("add $4, %esp");                    \
            iret();

void exception(uint32_t id) {

    Regs *regs;
    const static char *title[17] = {
        "DIVISION ERROR",
        "DEBUG EXCEPTION",
        "NMI",
        "BREAK POINT",
        "OVERFLOW",
        "BOUNDS CHECK",
        "INVALID OPCODE",
        "COPROCESSOR NOT VALID",
        "DOUBLE FAULT",
        "OVERRUN",
        "INVALID TSS",
        "SEGMENTATION NOT PRESENT",
        "STACK EXCEPTION",
        "GENERAL PROTECTION ERROR",
        "PAGE FAULT",
        "RESERVED EXCEPTION",
        "COPROCESSOR_ERROR"
    };

    cli();

    /* Get Registers: */
    __asm__("lea 12(%%ebp), %%eax":"=a"(regs));

    /* regular page fault? */
    if (id == 0x0E && page_fault(regs->err) == 0)
        return;

    /* Do Panic! */
    panic(regs, "Exception %d: %s.", id, title[id]);
    
}

void exp_gate() {
    __asm__("EXP00:");    exp(0x00, EXP_NO_ERR  );
    __asm__("EXP01:");    exp(0x01, EXP_NO_ERR  );
    __asm__("EXP02:");    exp(0x02, EXP_NO_ERR  );
    __asm__("EXP03:");    exp(0x03, EXP_NO_ERR  );
    __asm__("EXP04:");    exp(0x04, EXP_NO_ERR  );
    __asm__("EXP05:");    exp(0x05, EXP_NO_ERR  );
    __asm__("EXP06:");    exp(0x06, EXP_NO_ERR  );
    __asm__("EXP07:");    exp(0x07, EXP_NO_ERR  );
    __asm__("EXP08:");    exp(0x08, EXP_WITH_ERR);
    __asm__("EXP09:");    exp(0x09, EXP_NO_ERR  );
    __asm__("EXP0A:");    exp(0x0A, EXP_WITH_ERR);
    __asm__("EXP0B:");    exp(0x0B, EXP_WITH_ERR);
    __asm__("EXP0C:");    exp(0x0C, EXP_WITH_ERR);
    __asm__("EXP0D:");    exp(0x0D, EXP_WITH_ERR);
    __asm__("EXP0E:");    exp(0x0E, EXP_WITH_ERR);
    __asm__("EXP0F:");    exp(0x0F, EXP_NO_ERR  );
    __asm__("EXP10:");    exp(0x10, EXP_WITH_ERR);
}

void exception_init() {

    /* Initialize IDT Exception Descriptors: */
    uint32_t offset[0x11], i;

    __asm__("movl $EXP00, %%eax":"=a"(offset[0x00]));
    __asm__("movl $EXP01, %%eax":"=a"(offset[0x01]));
    __asm__("movl $EXP02, %%eax":"=a"(offset[0x02]));
    __asm__("movl $EXP03, %%eax":"=a"(offset[0x03]));
    __asm__("movl $EXP04, %%eax":"=a"(offset[0x04]));
    __asm__("movl $EXP05, %%eax":"=a"(offset[0x05]));
    __asm__("movl $EXP06, %%eax":"=a"(offset[0x06]));
    __asm__("movl $EXP07, %%eax":"=a"(offset[0x07]));
    __asm__("movl $EXP08, %%eax":"=a"(offset[0x08]));
    __asm__("movl $EXP09, %%eax":"=a"(offset[0x09]));
    __asm__("movl $EXP0A, %%eax":"=a"(offset[0x0A]));
    __asm__("movl $EXP0B, %%eax":"=a"(offset[0x0B]));
    __asm__("movl $EXP0C, %%eax":"=a"(offset[0x0C]));
    __asm__("movl $EXP0D, %%eax":"=a"(offset[0x0D]));
    __asm__("movl $EXP0E, %%eax":"=a"(offset[0x0E]));
    __asm__("movl $EXP0F, %%eax":"=a"(offset[0x0F]));
    __asm__("movl $EXP10, %%eax":"=a"(offset[0x10]));

    for (i = 0; i < 0x11; i++) {
        idt[i].offset_lo = offset[i]&0xFFFF;
        idt[i].selector = GDT_SEGMENT_SELECTOR(GDT_ENTRY_KERNEL_CODE);
        idt[i].etype = IDT_ETYPE_INTERRUPT;
        idt[i].dpl = PL_USER;
        idt[i].present = IDT_PRESENT;
        idt[i].offset_hi = offset[i]>>16;
    }

}
