#
#        +----------------------------------------------------------+
#        | +------------------------------------------------------+ |
#        | |  Quafios Boot-Loader.                                | |
#        | |  -> Quafios MBR.                                     | |
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
# This is the source code for Quafios Master Boot Record. When Quafios
# is installed on an MBR-partitioned medium, this MBR program should be
# installed to the MBR sector so that the disk becomes bootable. Quafios
# should be installed on a primary partition with boot flag set, so that
# this MBR program can detect it and boot from it.

.text
.code16
.global start

start:
    # DL stores boot disk number. if DL = 0xFF, then EDI stores the
    # ramdisk offset, and EBP stores image size.
    xor  %ax, %ax
    mov  %ax, %ds
    mov  %ax, %es
    mov  %ax, %ss
    mov  %ax, %sp

    # copy to 0x0000:0x0600
    push %di
    mov  $0x0100, %cx
    mov  $0x7C00, %si
    mov  $0x0600, %di
    rep  movsw
    pop  %di
    jmp  $0x0000, $lookup

lookup:
    # lookup partition table for bootable entry
    mov  $parttable, %si
    mov  $4, %cx
1:  cmpb $0x80, (%si)
    je   found
    add  $0x10, %si
    loop 1b

error:
    # print error message
    mov  $0x03, %ah
    xor  %bh, %bh
    int  $0x10        # get cursor position
    mov  $0x1301, %ax
    mov  $0x000C, %bx
    mov  $(msg_end-msg-1), %cx
    mov  $msg, %bp
    int  $0x10        # ega+ print string routine
2:  cli
    hlt

found:
    mov  8(%si), %ax
    mov  %ax, dap+8
    mov  10(%si), %ax
    mov  %ax, dap+10
    # check if ramdisk or normal disk
    cmp  $0xFF, %dl
    je   load_ramdisk
    # load partition's vbr from disk
    push %dx
    push %si
    mov  $0x42, %ah
    mov  $dap, %si
    int  $0x13
    pop  %si
    pop  %dx
    jmp  $0x0000, $0x7C00

load_ramdisk:
    # Requires unreal mode. actually if boot medium
    # is ramdisk, this means that another earlier
    # stage was already executed and it must have
    # entered unreal mode.
    push %edi
    push %si
    mov  dap+2, %cx
    shl  $9, %cx
    mov  dap+4, %si
    mov  dap+8, %eax
    shl  $9, %eax
    add  %eax, %edi
1:  mov  (%edi), %al
    mov  %al, (%si)
    inc  %edi
    inc  %si
    loop 1b
    pop  %si
    pop  %edi
    jmp  $0x0000, $0x7C00

dap:        .byte 16
            .byte 0
            .word 1 # number of sectors to read
            .long 0x00007C00  # segment:offset to load
            .quad 0 # lba will be copied here

msg:        .string "Quafios can't find a bootable partition.\r\n"
msg_end:

freespace:  .org 0x1BE

parttable:  .space 64 # partition table is here

signature:  .word 0xAA55
