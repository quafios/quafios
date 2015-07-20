/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Kernel error codes header.                       | |
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

#ifndef ERROR_H
#define ERROR_H

#include <arch/type.h>

typedef int32_t error_t;

#define ESUCCESS        0x00
#define EBUSY           0x01
#define EFAULT          0x02
#define ENODEV          0x03
#define ENOENT          0x04
#define ENOMEM          0x05
#define ENOTSF          0x06
#define ENOTDIR         0x07
#define EINVAL          0x08
#define EEXIST          0x09
#define ENOTEMPTY       0x0A
#define EREADONLY       0x0B
#define ENOSPC          0x0C
#define EISDIR          0x0D
#define EXDEV           0x0E
#define EMFILE          0x0F
#define EBADF           0x10
#define EIO             0x11

#endif
