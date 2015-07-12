/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios C Standard Library.                         | |
 *        | |  -> stdarg header.                                   | |
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

#ifndef STDARG_H
#define STDARG_H

/* variable arguments handling. */
typedef void * va_list; /* I implement it as a void * for simplicity... */

#define ARGSIZ 4

/* macros: */
#define va_start(ap, paramN) (__extension__({           \
            ap = (void *)(&paramN);                     \
            int size= (sizeof(paramN)+ARGSIZ-1)/ARGSIZ; \
            ap += size*ARGSIZ;                          \
        }))
#define va_end(ap) {ap = NULL}
#define va_arg(ap, type) (type)(__extension__({         \
            type __ret = *((type *) ap);                \
            /* update ap */                             \
            int size= (sizeof(type)+ARGSIZ-1)/ARGSIZ;   \
            ap += size * ARGSIZ;                        \
            __ret; /* return the argument */            \
	}))

#endif
