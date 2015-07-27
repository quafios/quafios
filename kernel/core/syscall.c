/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> syscall() procedure.                             | |
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
#include <sys/mm.h>
#include <sys/syscall.h>
#include <sys/error.h>
#include <sys/semaphore.h>

#define DO_CALL(func_name) func_name(arg[1], arg[2], arg[3], arg[4], arg[5])
#define DO_CALL2(func_name, args_type) func_name(*((args_type *) arg[1]))

int32_t syscall(int32_t number, ...) {

    int32_t *arg = &number;
    int32_t ret;

    switch (number) {
        case SYS_MOUNT:     {ret=DO_CALL(mount            ); break;}
        case SYS_UMOUNT:    {ret=DO_CALL(umount           ); break;}
        case SYS_MKNOD:     {ret=DO_CALL(mknod            ); break;}
        case SYS_RENAME:    {ret=DO_CALL(rename           ); break;}
        case SYS_LINK:      {ret=DO_CALL(link             ); break;}
        case SYS_UNLINK:    {ret=DO_CALL(unlink           ); break;}
        case SYS_MKDIR:     {ret=DO_CALL(mkdir            ); break;}
        case SYS_RMDIR:     {ret=DO_CALL(rmdir            ); break;}
        case SYS_OPEN:      {ret=DO_CALL(open             ); break;}
        case SYS_CLOSE:     {ret=DO_CALL(close            ); break;}
        case SYS_CHDIR:     {ret=DO_CALL(chdir            ); break;}
        case SYS_GETCWD:    {ret=DO_CALL(getcwd           ); break;}
        case SYS_TRUNCATE:  {ret=DO_CALL(truncate         ); break;}
        case SYS_FTRUNCATE: {ret=DO_CALL(ftruncate        ); break;}
        case SYS_READ:      {ret=DO_CALL(read             ); break;}
        case SYS_WRITE:     {ret=DO_CALL(write            ); break;}
        case SYS_SEEK:      {ret=DO_CALL(seek             ); break;}
        case SYS_READDIR:   {ret=DO_CALL(readdir          ); break;}
        case SYS_STAT:      {ret=DO_CALL(stat             ); break;}
        case SYS_FSTAT:     {ret=DO_CALL(fstat            ); break;}
        case SYS_STATFS:    {ret=DO_CALL(statfs           ); break;}
        case SYS_FSTATFS:   {ret=DO_CALL(fstatfs          ); break;}
        case SYS_DUP:       {ret=DO_CALL(dup              ); break;}
        case SYS_DUP2:      {ret=DO_CALL(dup2             ); break;}
        case SYS_IOCTL:     {ret=DO_CALL(ioctl            ); break;}
        case SYS_EXECVE:    {ret=DO_CALL(execve           ); break;}
        case SYS_FORK:      {ret=DO_CALL(fork             ); break;}
        case SYS_WAITPID:   {ret=DO_CALL(waitpid          ); break;}
        case SYS_EXIT:      {ret=DO_CALL(exit             ); break;}
        case SYS_BRK:       {ret=DO_CALL(brk              ); break;}
        case SYS_MMAP:      {ret=DO_CALL2(mmap, mmap_arg_t); break;}
        case SYS_SEND:      {ret=DO_CALL(send             ); break;}
        case SYS_RECEIVE:   {ret=DO_CALL(receive          ); break;}
        case SYS_GETPID:    {ret=DO_CALL(getpid           ); break;}
        case SYS_REBOOT:    {ret=DO_CALL(legacy_reboot    ); break;}
        case SYS_MUNMAP:    {ret=DO_CALL(munmap           ); break;}
        default:            {ret=-EINVAL                   ; break;}
    }

    return ret;

}

