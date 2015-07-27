#
#        +----------------------------------------------------------+
#        | +------------------------------------------------------+ |
#        | |  Quafios Boot-Loader.                                | |
#        | |  -> multiboot module.                                | |
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

# This file is used for boot Quafios ISO image using multiboot-compliant
# boot loader like GRUB.

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

buf:
.space 2048

.text
.code32
.global main

###########################
#   READ SECTOR ROUTINE   #
###########################

readsect:
    # read sector from ISO image in "buf"
    # EAX <-- SECTOR NUMBER
    # EDI <-- destination
    pusha
    mov mod1_start, %esi
    shl $11, %eax
    add %eax, %esi
    mov $2048, %ecx
    rep movsb
    popa
    ret

###########################
#      MAIN ROUTINE       #
###########################

main:

# Read Modules:
# --------------
    mov  %ebx, %esi
    movl 24(%esi), %esi # modules address.
    mov  0(%esi), %eax
    mov  %eax, mod1_start
    mov  4(%esi), %eax
    mov  %eax, mod1_end

# Search for boot record:
# ------------------------
    mov  $0x0F, %eax
    mov  $buf, %edi
1:  inc  %eax
    call readsect
    cmpb $0x00, buf+0
    jne  1b
    cmpb $'C', buf+1
    jne  1b
    cmpb $'D', buf+2
    jne  1b
    cmpb $'0', buf+3
    jne  1b
    cmpb $'0', buf+4
    jne  1b
    cmpb $'1', buf+5
    jne  1b
     # boot record is actually at sector 17
    mov  buf+0x47, %eax # boot catalog

# Read the initial/default entry in boot catalog:
# ------------------------------------------------
1:  call readsect
    xor  %ebx, %ebx
2:  cmpb $0x88, buf(%ebx)
    je   3f
    add  $0x20, %ebx
    cmp  $0x800, %ebx
    jne  2b
    inc  %eax
    jmp  1b
3:  mov  buf+8(%ebx), %eax # lba of the boot sector

# Load boot sector to 0x7C00:
# ----------------------------
    mov  $0x7C00, %edi
    call readsect

# Copy ISO image:
# ------------------
    mov mod1_start, %esi
    mov $0x2000000, %edi
2:
    mov (%esi), %al
    mov %al, (%edi)
    inc %esi
    inc %edi
    cmp mod1_end, %esi
    jne 2b

# load information about ram disk:
# ---------------------------------
    mov mod1_end, %ebp
    sub mod1_start, %ebp
    mov $0x2000000, %edi
    mov $0xFF, %edx

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
    ljmp $0x0000, $0x7C00
