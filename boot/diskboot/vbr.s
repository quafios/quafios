#
#        +----------------------------------------------------------+
#        | +------------------------------------------------------+ |
#        | |  Quafios Boot-Loader.                                | |
#        | |  -> Quafios VBR.                                     | |
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
# This is the source code for Quafios Volume Boot Record. When MBR
# starts execution, it expects that Quafios is installed on a bootable
# primary partition. The MBR scans the partition table to find such partition,
# then loads the VBR of the partition to 0x7C00. This program (vbr.s) is
# the VBR for Quafios partition. It takes control from MBR and starts
# looking for /boot/loader.bin on the partition, then loads it to 0x8000.

.text
.code16
.global start

.set DISKFS_BLOCK_SIZE,  0x1010  # 2 bytes
.set DISKFS_INODE_START, 0x103A  # 4 bytes
.set DISKFS_BLOCK_START, 0x1042  # 4 bytes

.set INODE_POOL,         0x2000

.set PTRS_PER_BLOCK,     0x3000  # 2 bytes
.set BLOCK_MASK,         0x3002  # 2 bytes
.set BLOCK_SHIFT2,       0x3004  # 2 bytes
.set BLOCK_SHIFT3,       0x3006  # 2 bytes
.set LEVEL0_BLOCKS,      0x3008  # 4 bytes
.set LEVEL01_BLOCKS,     0x300C  # 4 bytes
.set LEVEL012_BLOCKS,    0x3010  # 4 bytes
.set LEVEL0_OFF,         0x3014  # 2 bytes
.set LEVEL1_OFF,         0x3016  # 2 bytes
.set LEVEL2_OFF,         0x3018  # 2 bytes
.set LEVEL3_OFF,         0x301A  # 2 bytes

start:
    # DL stores boot disk number. if DL = 0xFF, then EDI stores the
    # ramdisk offset, and EBP stores image size.
    # DS:SI stores a pointer to partition entry.
    xor  %ax, %ax
    mov  %ax, %es
    mov  %ax, %ss
    mov  %ax, %sp
    # store dl and ramdisk offset
    mov  %dl, drivenum
    cmp  $0xFF, %dl
    jne  1f
    mov  %edi, ramdiskoff
    mov  %ebp, ramdisksize
    # load & store partition start lba
1:  mov  8(%si),  %bx
    mov  10(%si), %cx
    mov  %ax, %ds
    mov  %bx, partstart+0
    mov  %cx, partstart+2
    # load second sector of VBR
    add  $1, %bx
    adc  $0, %cx
    mov  %bx, dap+8
    mov  %cx, dap+10
    call readsect
    # load superblock
    add  $1, %bx
    adc  $0, %cx
    movw $0x01, dap+2
    movw $0x1000, dap+4
    mov  %bx, dap+8
    mov  %cx, dap+10
    call readsect
    # calculate disk parameters
    mov  $0x3000, %si
    mov  DISKFS_BLOCK_SIZE, %ax
    shr  $2, %ax
    mov  %ax, (%si)
    dec  %ax
    mov  %ax, 2(%si)
    xor  %cx, %cx
1:  inc  %cx
    shr  $1,  %ax
    jne  1b
    mov  %cx, 4(%si)  # log2(PTRS_PER_BLOCK)
    shl  $1,  %cx
    mov  %cx, 6(%si)
    shr  $1,  %cx
    mov  $0x000C, %ax
    xor  %bx, %bx
    mov  %ax, 8(%si)
    mov  %bx, 10(%si)
    mov  %ax, 12(%si)
    mov  %bx, 14(%si)
    mov  %ax, 16(%si)
    mov  %bx, 18(%si)
1:  shl  $1, %ax
    rcl  $1, %bx
    loop 1b
    add  %ax, 12(%si)
    adc  %bx, 14(%si)
    mov  BLOCK_SHIFT2, %cx
1:  shl  $1, %ax
    rcl  $1, %bx
    loop 1b
    add  %ax, 16(%si)
    adc  %bx, 18(%si)
    # load inode 2 (root directory) structure
    mov  $2, %ax
    xor  %dx, %dx
    call loadinode
    # now si contains a pointer for the inode, lookup for boot/
    mov  $boot, %di
    call lookup
    # load boot inode
    call loadinode
    # now si contains a pointer for the inode
    mov  $loaderbin, %di
    call lookup
    # load loader.bin inode
    call loadinode
    # load the whole file
    call loadfile
    # jmp to boot loader (loader.bin).
    jmp  $0x0000, $0x8000

readsect:
    # dap should be ready
    pusha
    mov  drivenum, %dl
    cmp  $0xFF, %dl
    je   load_ramdisk
    mov  $0x42, %ah
    mov  $dap, %si
    int  $0x13
    popa
    ret

load_ramdisk:
    # Load sector from ramdisk
    # Requires unreal mode. actually if boot medium
    # is ramdisk, this means that another earlier
    # stage was already executed and it must have
    # entered unreal mode.
    push %edi
    push %eax
    mov  ramdiskoff, %edi
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
    pop  %eax
    pop  %edi
    popa
    ret

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

boot:       .string "boot"
loaderbin:  .string "loader.bin"
msg:        .string "Quafios can't find /boot/loader.bin"
msg_end:

dap:         .byte 16
             .byte 0
             .word 1 # number of sectors to read
             .long 0x00007E00 # segment:offset to load
             .quad 0 # lba will be copied here

             .org  0x1F0 # disk boot parameters
drivenum:    .byte 0
ramdiskoff:  .long 0
ramdisksize: .long 0
partstart:   .long 0
fstype:      .byte 0

freespace:  .org 0x1FE
signature:  .word 0xAA55

loadfile:
    # input: %si (inode structure location)
    # loads file to 0x8000
    push %ax
    push %dx
    push %si
    push %di
    mov  %si, %di
    mov  $0x8000, %si
    xor  %ax, %ax
    xor  %dx, %dx
1:  call loadfileblk
    add  DISKFS_BLOCK_SIZE, %si
    add  $1, %ax
    adc  $0, %dx
    cmp  16(%di), %ax
    jne  1b
    cmp  18(%di), %dx
    jne  1b
    # done
    pop  %di
    pop  %si
    pop  %dx
    pop  %ax
    ret

strcmp:
    # si+4: string 1
    # di: string 2
    # cx: result of comparison
    push %si
    push %di
    add  $4, %si
    xor  %cx, %cx
1:  cmpsb
    jne  2f
    cmpb $0, (%si)
    jne  1b
    cmpb $0, (%di)
    je   3f
2:  mov  $1, %cx
3:  pop  %di
    pop  %si
    ret

lookup:
    # input:   %di (pointer to file name)
    #          %si (inode structure location)
    # returns: %dx:%ax (inode of the file that has the same name of (%di))
    push %bx
    push %cx
    push %si
    xor  %ax,%ax # block number
    xor  %dx,%dx # block number
    mov  $0x8000, %bx
    add  DISKFS_BLOCK_SIZE, %bx
    mov  %si, %bp
nextlbk:
    mov  $0x8000, %si
    push %di
    mov  %bp, %di
    call loadfileblk
    pop  %di
nextentry:
    cmpw $0, 0(%si)
    jne  1f
    cmpw $0, 2(%si)
    je   error # last entry in the directory
1:  cmpw $0, 0(%si)
    jne  2f
    cmpw $1, 2(%si)
    je   trynext # skip deleted entry
2:  call strcmp
    cmp  $0, %cx
    je   found
trynext:
    add  $0x20, %si
    cmp  %si, %bx
    jne  nextentry
    # fetch another block
    add  $1, %ax
    adc  $0, %dx
    jmp  nextlbk
found:
    mov  0(%si), %ax
    mov  2(%si), %dx
    pop  %si
    pop  %cx
    pop  %bx
    ret

loadfileblk:
    # load a block of some file
    # input:   si    memory location
    #          di    inode structure location
    #          dx:ax block number in the file
    # calculate ptrs per block
    pusha
    mov  $0x3000, %bx

lvl1:
    cmp  10(%bx), %dx
    ja   lvl2
    cmp  8(%bx), %ax
    jae  lvl2
    movw %ax, 20(%bx)
    mov  $1, %cx
    jmp  read_blocks

lvl2:
    cmp  14(%bx), %dx
    ja   lvl3
    cmp  12(%bx), %ax
    jae  lvl3
    movw $0x0C, 20(%bx)
    sub  8(%bx), %ax
    movw %ax,   22(%bx)
    mov  $2, %cx
    jmp  read_blocks

lvl3:
    cmp  18(%bx), %dx
    ja   lvl4
    cmp  16(%bx), %ax
    jae  lvl4
    sub  12(%bx), %ax
    sbb  14(%bx), %dx
    movw $0x0D, 20(%bx)
    push %ax
    and  2(%bx), %ax
    movw %ax, 22(%bx)
    pop  %ax
    mov  4(%bx), %cx
1:  shr  $1, %dx
    rcr  $1, %ax
    loop 1b
    and  2(%bx), %ax
    movw %ax, 24(%bx)
    mov  $3, %cx
    jmp  read_blocks

lvl4:
    sub  16(%bx), %ax
    sbb  18(%bx), %dx
    movw $0x0E, 20(%bx)
    mov  %ax, %cx
    and  2(%bx), %cx
    movw %cx, 22(%bx)
    mov  4(%bx), %cx
1:  shr  $1, %dx
    rcr  $1, %ax
    loop 1b
    mov  %ax, %cx
    and  2(%bx), %cx
    movw %cx, 24(%bx)
    mov  4(%bx), %cx
1:  shr  $1, %dx
    rcr  $1, %ax
    loop 1b
    and  2(%bx), %ax
    movw %ax, 26(%bx)
    mov  $4, %cx

read_blocks:
    add  $24, %di
    mov  $LEVEL0_OFF, %bx
1:  mov  (%bx), %ax
    shl  $2, %ax
    add  %ax, %di
    push %bx
    push %cx
    mov  0(%di), %bx
    mov  2(%di), %cx
    push %si
    call loadcluster
    pop  %si
    pop  %cx
    pop  %bx
    mov  %si, %di
    add  $2, %bx
    loop 1b
    popa
    ret

loadinode:
    # load inode structure from disk
    # dx:ax inode number
    # returns si, pointer to inode
    # dividing (dx:ax) by inodes_per_block gives inode block number
    # as an offset to the first block in inodes area.
    # inodes per block = block size / inode size
    # as block size is 16-bit field, we can simply divide dx:ax
    # by block_size>>7
    push %ax
    push %bx
    push %cx
    push %dx
    mov  DISKFS_BLOCK_SIZE, %bx
    shr  $7, %bx
    div  %bx
    shl  $7, %dx
    # now %ax contains block number, %dx contains offset of the inode
    # in the block
    mov  %ax, %bx
    xor  %cx, %cx
    add  DISKFS_INODE_START+0, %bx
    adc  DISKFS_INODE_START+2, %cx
    mov  $0x2000, %si
    call loadcluster
    add  %dx, %si
    pop  %dx
    pop  %cx
    pop  %bx
    pop  %ax
    ret

loadcluster:
    # load cluster routine
    # si --> memory location
    # cx:bx --> cluster number
    pusha
    mov  DISKFS_BLOCK_SIZE, %ax
    shr  $9, %ax
    push %ax
    push %ax
    mul  %bx
    mov  %ax, %bx
    mov  %dx, %bp
    pop  %ax
    mul  %cx
    add  %bp, %ax
    # now actual sector number is at ax:bx
    add  partstart+0, %bx
    adc  partstart+2, %ax
    add  $2, %bx
    adc  $0, %ax
    pop  %cx
    mov  %cx, dap+2
    mov  %si, dap+4
    mov  %bx, dap+8
    mov  %ax, dap+10
    call readsect
    popa
    ret

.org 0x400, 0
