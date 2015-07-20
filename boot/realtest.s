#
#        +----------------------------------------------------------+
#        | +------------------------------------------------------+ |
#        | |  Quafios Boot-Loader.                                | |
#        | |  -> Testing code.                                    | |
#        | +------------------------------------------------------+ |
#        +----------------------------------------------------------+
#
# This file is part of Quafios 2.0.1 source code.
# Copyright (C) 2015  Mostafa Abd El-Aziz Mohamed.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Quafios.  If not, see <http://www.gnu.org/licenses/>.
#
# Visit http://www.quafios.com/ for contact information.
#

# This file is used for testing Quafios
# on my real machine.

.set MULTIBOOT_HEADER_MAGIC, 0x1BADB002
.set MULTIBOOT_HEADER_FLAGS, 0x00000000

# multiboot header :D
.section .mboot, "a"
.long MULTIBOOT_HEADER_MAGIC
.long MULTIBOOT_HEADER_FLAGS
.long -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

.data

gdt:
.quad 0x0000000000000000 # Null Descriptor
.quad 0x008F9A000000FFFF # 16-bit Code Flat Descriptor
.quad 0x008F92000000FFFF # 16-bit Data Flat Descriptor

gdtr:
.word 0x0017 # 3 descriptors --> size=3*8=24, limit=size-1=23=0x17
.long gdt

idtr:
.word 0x03FF
.long 0x00000000

mod1_start:
.long 0

mod1_end:
.long 0

mod2_start:
.long 0

mod2_end:
.long 0

.text
.code32
.global main
main:

# Boot Sector Parameters:
# -------------------------
    movl $0x10, 0x7C08 # primary volume descriptor
    movb $0x02, 0x7DF0 # boot type (debug).

# Read Modules:
# --------------
    mov %ebx, %esi
    movl 24(%esi), %esi # modules address.
    mov 0(%esi), %eax
    mov %eax, mod1_start
    mov 4(%esi), %eax
    mov %eax, mod1_end
    mov 16(%esi), %eax
    mov %eax, mod2_start
    mov 20(%esi), %eax
    mov %eax, mod2_end

# Copy boot-loader:
# ------------------
    mov mod1_start, %esi
    add $2048, %esi
    mov $0x8000, %edi
1:
    mov (%esi), %al
    mov %al, (%edi)
    inc %esi
    inc %edi
    cmp mod1_end, %esi
    jne 1b

# Copy ISO image:
# ------------------
    mov mod2_start, %esi
    mov $0x2000000, %edi
2:
    mov (%esi), %al
    mov %al, (%edi)
    inc %esi
    inc %edi
    cmp mod2_end, %esi
    jne 2b

# Enter Unreal Mode:
# -----------------

    # load gdt register
    lgdt gdtr

    # load idt register
    lidt idtr

    # set CS to 16-bit code:
    ljmp $0x0008, $_16bit

    # set remaining registers to 16-bit:
.code16
_16bit:
    mov $0x0010, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    # enter unreal mode:
    mov %cr0, %eax
    and $0xFE, %al
    mov %eax, %cr0

    # reset registers:
    mov $0x0000, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    # zeroise the 32-bit stack pointer:
    mov $0x0000, %esp

    # jmp to boot loader:
    ljmp $0x0000, $0x8000
