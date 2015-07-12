/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> syscall() procedure.                             | |
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

#include <arch/type.h>
#include <sys/mm.h>
#include <sys/syscall.h>
#include <sys/error.h>

#define DO_CALL(func_name) func_name(arg[1], arg[2], arg[3], arg[4], arg[5])
#define DO_CALL2(func_name, args_type) func_name(*((args_type *) arg[1]))

int32_t syscall(int32_t number, ...) {

    int32_t *arg = &number;

    switch (number) {

        case SYS_MOUNT:     return DO_CALL(mount    );
        case SYS_UMOUNT:    return DO_CALL(umount   );
        case SYS_MKNOD:     return DO_CALL(mknod    );
        case SYS_RENAME:    return DO_CALL(rename   );
        case SYS_LINK:      return DO_CALL(link     );
        case SYS_UNLINK:    return DO_CALL(unlink   );
        case SYS_MKDIR:     return DO_CALL(mkdir    );
        case SYS_RMDIR:     return DO_CALL(rmdir    );
        case SYS_OPEN:      return DO_CALL(open     );
        case SYS_CLOSE:     return DO_CALL(close    );
        case SYS_CHDIR:     return DO_CALL(chdir    );
        case SYS_GETCWD:    return DO_CALL(getcwd   );
        case SYS_TRUNCATE:  return DO_CALL(truncate );
        case SYS_FTRUNCATE: return DO_CALL(ftruncate);
        case SYS_READ:      return DO_CALL(read     );
        case SYS_WRITE:     return DO_CALL(write    );
        case SYS_SEEK:      return DO_CALL(seek     );
        case SYS_READDIR:   return DO_CALL(readdir  );
        case SYS_STAT:      return DO_CALL(stat     );
        case SYS_FSTAT:     return DO_CALL(fstat    );
        case SYS_STATFS:    return DO_CALL(statfs   );
        case SYS_FSTATFS:   return DO_CALL(fstatfs  );
        case SYS_DUP:       return DO_CALL(dup      );
        case SYS_DUP2:      return DO_CALL(dup2     );
        case SYS_IOCTL:     return DO_CALL(ioctl    );
        case SYS_EXECVE:    return DO_CALL(execve   );
        case SYS_FORK:      return DO_CALL(fork     );
        case SYS_WAITPID:   return DO_CALL(waitpid  );
        case SYS_EXIT:      return DO_CALL(exit     );
        case SYS_BRK:       return DO_CALL(brk      );
        case SYS_MMAP:      return DO_CALL2(mmap, mmap_arg_t);
        case SYS_SEND:      return DO_CALL(send     );
        case SYS_RECEIVE:   return DO_CALL(receive  );
        case SYS_GETPID:    return DO_CALL(getpid   );
        default:            return -EINVAL;

    }

}

