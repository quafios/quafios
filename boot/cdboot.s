#
#        +----------------------------------------------------------+
#        | +------------------------------------------------------+ |
#        | |  Quafios Boot-Loader.                                | |
#        | |  -> EL-TORITO boot sector.                           | |
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

###############################
#    ISO Boot Loader Entry    #
###############################

.text
.code16
.global _start

_start:
jmp  step_i

.org 8

.global bi_PrimaryVolumeDescriptor
bi_PrimaryVolumeDescriptor: .long  0  # LBA of the Primary Volume Descriptor
bi_BootFileLocation:        .long  0  # LBA of the Boot File
bi_BootFileLength:          .long  0  # Length of the boot file in bytes
bi_Checksum:                .long  0  # 32 bit checksum
bi_Reserved:                .space 40 # Reserved 'for future standardization'

dap: # disk address packet
.byte 0x10 # dap size
.byte 0x00 # unused
.word 0x01 # number of sectors.
.word 0x00 # offset
.word 0x00 # segment
.quad 0x00 # LBA

    # I) Setup Registers & Stacks:
    # -----------------------------
step_i:
    ljmp $0,  $1f
1:  xor  %ax, %ax
    mov  %ax, %ds
    mov  %ax, %es
    mov  %ax, %ss
    mov  %ax, %sp

    # II) Load the remainings of the boot file to memory:
    # ----------------------------------------------------
step_ii:
    # Store Drive Index
    mov %dl, drive_index

    # Load DAP
    movw $dap,                %si

    # calculate count of sectors to read.
    movl bi_BootFileLength,   %eax
    addl $0x7FF,              %eax
    shrl $11,                 %eax
    decl %eax                         # skip this track.
    jz   step_iii
    movw %ax,                 2(%si)

    # 0x0000:0x8000
    movw $0x8000,             4(%si)
    movw $0x0000,             6(%si)

    #LBA
    movl bi_BootFileLocation, %eax
    incl %eax                         # skip this track.
    movl %eax,                8(%si)

    # READ
    movb $0x42,       %ah
    int  $0x13

    # III) Execute loader.bin:
    # -------------------------
step_iii:
.code16gcc
    mov $0, %esp
    call 0x8000 # main() of loader.bin
    jmp . # should never reach this place.

.org 0x1F0
# 0x7DF0
boot_type:   .byte 0x01  # EL-TORITO
# 0x7DF1
drive_index: .byte 0x00

.org 0x1FE
signature:   .word 0xAA55
.org 0x800 # ISO Sector is 2KB.
