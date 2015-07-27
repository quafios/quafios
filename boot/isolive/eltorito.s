#
#        +----------------------------------------------------------+
#        | +------------------------------------------------------+ |
#        | |  Quafios ISO-Live Bootstrap program.                 | |
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

.section .text.eltorito,"ax",@progbits
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

    # I) Setup segment registers:
    # ----------------------------
step_i:
    xor  %ax, %ax
    mov  %ax, %ds
    mov  %ax, %es
    mov  %ax, %ss
    mov  %ax, %sp

    # II) Copy to 0x0000:0x1000:
    # ----------------------------
step_ii:
    push %di
    mov  $0x0100, %cx
    mov  $0x7C00, %si
    mov  $0x1000, %di
    rep  movsw
    pop  %di
    jmp  $0x0000, $step_iii

    # III) Load the remainings of the boot file to memory:
    # -----------------------------------------------------
step_iii:
    # store drive index and ramdisk info (if any)
    mov  %dl,                 drivenum
    cmp  $0xFF,               %dl
    jne  1f
    mov  %edi,                ramdiskoff
    mov  %ebp,                ramdisksize

    # load DAP
1:  movw $dap,                %si

    # calculate count of sectors to read.
    movl bi_BootFileLength,   %eax
    addl $0x7FF,              %eax
    shrl $11,                 %eax
    decl %eax                         # skip this track.
    jz   step_iii
    movw %ax,                 2(%si)

    # 0x0000:0x1800
    movw $0x1800,             4(%si)
    movw $0x0000,             6(%si)

    #LBA
    movl bi_BootFileLocation, %eax
    incl %eax                         # skip this track.
    movl %eax,                8(%si)

    # dap is now ready
    pusha
    mov  drivenum, %dl
    cmp  $0xFF, %dl
    je   2f
    mov  $0x42, %ah
    mov  $dap, %si
    int  $0x13
    popa
    jmp  step_iv

    # Load sector from ramdisk
    # Requires unreal mode. actually if boot medium
    # is ramdisk, this means that another earlier
    # stage was already executed and it must have
    # entered unreal mode.
2:  push %edi
    push %eax
    mov  ramdiskoff, %edi
    mov  dap+2, %cx
    shl  $11, %cx
    mov  dap+4, %si
    mov  dap+8, %eax
    shl  $11, %eax
    add  %eax, %edi
3:  mov  (%edi), %al
    mov  %al, (%si)
    inc  %edi
    inc  %si
    loop 3b
    pop  %eax
    pop  %edi
    popa

    # IV) Jump to C main():
    # ----------------------
step_iv:
.code16gcc
    mov $0, %esp
    call main

             .org  0x1F0 # disc boot parameters
drivenum:    .byte 0
ramdiskoff:  .long 0
ramdisksize: .long 0
partstart:   .long 0
fstype:      .byte 0

.org 0x1FE
signature:   .word 0xAA55

.org 0x800
