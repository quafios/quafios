/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios C Standard Library.                         | |
 *        | |  -> API: Process management.                         | |
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

#include <api/proc.h>
#include <api/syscall.h>
#include <errno.h>

int fork() {
    int ret = syscall(SYS_FORK);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return ret;
}

void _exit(int status) {
    syscall(SYS_EXIT, status);
}

int waitpid(int pid, int *status) {
    int ret = syscall(SYS_WAITPID, pid, status);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

int send(int pid, msg_t *msg) {
    int ret = syscall(SYS_SEND, pid, msg);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

int receive(msg_t *msg, int wait) {
    int ret = syscall(SYS_RECEIVE, msg, wait);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

int getpid() {
    return syscall(SYS_GETPID);
}
