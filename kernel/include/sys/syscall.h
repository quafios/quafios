/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> System call header.                              | |
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

#ifndef SYSCALL_H
#define SYSCALL_H

/* Quafios Kernel System Call Table: */

#define SYS_MOUNT       0x00
#define SYS_UMOUNT      0x01
#define SYS_MKNOD       0x02
#define SYS_RENAME      0x03
#define SYS_LINK        0x04
#define SYS_UNLINK      0x05
#define SYS_MKDIR       0x06
#define SYS_RMDIR       0x07
#define SYS_OPEN        0x08
#define SYS_CLOSE       0x09
#define SYS_CHDIR       0x0A
#define SYS_GETCWD      0x0B
#define SYS_TRUNCATE    0x0C
#define SYS_FTRUNCATE   0x0D
#define SYS_READ        0x0E
#define SYS_WRITE       0x0F
#define SYS_SEEK        0x10
#define SYS_READDIR     0x11
#define SYS_STAT        0x12
#define SYS_FSTAT       0x13
#define SYS_STATFS      0x14
#define SYS_FSTATFS     0x15
#define SYS_DUP         0x16
#define SYS_DUP2        0x17
#define SYS_IOCTL       0x18
#define SYS_EXECVE      0x19
#define SYS_FORK        0x1A
#define SYS_WAITPID     0x1B
#define SYS_EXIT        0x1C
#define SYS_BRK         0x1D
#define SYS_MMAP        0x1E
#define SYS_SEND        0x1F
#define SYS_RECEIVE     0x20
#define SYS_GETPID      0x21
#define SYS_REBOOT      0x22
#define SYS_MUNMAP      0x23

#endif
