/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios ISO-Live Bootstrap program.                 | |
 *        | |  -> Chain loader.                                    | |
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

extern uint32_t output_start;
extern uint32_t output_end;

void chainloader() {
    int32_t i;
    uint32_t size = output_end-output_start;
    for (i = 0; i < 0x200; i++)
        ((uint8_t *) 0x7C00)[i] = ((uint8_t *) 0x1000000)[i];
    __asm__("mov %%eax, %%ebp;\n"
            "jmp $0, $0x7C00"::"d"(0xFF), "D"(0x1000000), "a"(size));
}
