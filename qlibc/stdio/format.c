/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios C Standard Library.                         | |
 *        | |  -> Standard I/O: Formatted I/O.                     | |
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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* Definitions:  */
/* ------------- */
#define PUTC(CH) {                                      \
            if (__io_char(stream, str, which, CH,       \
                          1 /* write */, n, n_is_valid, \
                          &count) == EOF)               \
            return count;                               \
        }

#define PUTC_SUB(CH) {                                  \
            if (__io_char(stream, str, which, CH,       \
                          1 /* write */, n, n_is_valid, \
                          count) == EOF)                \
            return 1;                                   \
        }

#define GETC() __io_char(stream, str, which, 0,0/* read */,0,0,&count);

#define GETC_SUB() __io_char(stream, str, which, 0,0/* read */,0,0,count);

#define UNGETC(CH) {                                    \
            int __ret;                                  \
            if (which == 0 /* stream */) {              \
                if (CH != EOF)                          \
                    count--;                            \
                __ret = ungetc(CH, stream);             \
            } else /* string */ {                       \
                count--;                                \
                __ret = CH;                             \
            }                                           \
            __ret;                                      \
        }

#define UNGETC_SUB(CH) {                                \
            int __ret;                                  \
            if (which == 0 /* stream */) {              \
                if (CH != EOF)                          \
                    (*count)--;                         \
                __ret = ungetc(CH, stream);             \
            } else /* string */ {                       \
                (*count)--;                             \
                __ret = CH;                             \
            }                                           \
            __ret;                                      \
        }

#define F_LEFTJUSTIFY   1
#define F_FORCESIGN     2 /* force sign to be shown */
#define F_SPACE         4 /* space if no sign */
#define F_HASH          8
#define F_ZEROPAD       16

#define LEN_NONE        0
#define LEN_HH          1
#define LEN_H           2
#define LEN_L           3
#define LEN_LL          4
#define LEN_J           5
#define LEN_Z           6
#define LEN_T           7
#define LEN_LCAP        8

/****************************************************************************/
/*                               __io_char()                                */
/****************************************************************************/

int __io_char(FILE *stream, char *str, int which, char c, int dir,
              size_t n, int n_is_valid, size_t *count) {

    int ret;

    if (n_is_valid && *count >= n) {
        /* n characters or more has been already accessed. */
        (*count)++;
        return c;
    }

    if (which == 0) {
        /* stream... */
        if (dir == 0) {
            /* read */
            ret = fgetc(stream);
        } else {
            /* write */
            ret = fputc(c, stream);
        }
    } else {
        /* string */
        if (dir == 0) {
            /* read */
            ret = str[*count];
        } else {
            /* write */
            ret = str[*count] = c;
        }
    }

    if (ret != EOF)
        (*count)++;

    return ret;

}

/****************************************************************************/
/*                             parseSignedInt()                             */
/****************************************************************************/

int parseSignedInt(FILE *stream, char *str, int which, size_t n,
                   int n_is_valid, va_list *arg, size_t *count, char spec,
                   char flags, int width, int precision, int length) {

    char buffer[100];
    int bufi = 0;
    char sign = 0;
    __extension__ long long num;

    /* Counters: */
    int before; /* spaces before prefix                   */
    int prefix; /* count of prefix characters (i.e, sign) */
    int digits; /* count digits that will be printed      */
    int after;  /* spaces after prefix                    */

    if (precision >= 0)
        /* precision is specified */
        flags &= ~F_ZEROPAD; /* ignore 0 flag. */
    else
        /* negative value of precision */
        precision = 1; /* default value */

    /* I) Detect Integer Type:  */
    /* ------------------------ */
    switch(length) {
        case LEN_NONE:
        num = va_arg(*arg, int);
        break;

        case LEN_HH:
        num = va_arg(*arg, char);
        break;

        case LEN_H:
        num = va_arg(*arg, short);
        break;

        case LEN_L:
        num = va_arg(*arg, long);
        break;

        case LEN_LL:
        case LEN_LCAP:
        num = __extension__ va_arg(*arg, long long);
        break;

        case LEN_J:
        /* TODO: intmax_t.. */
        num = 0;
        break;

        case LEN_Z:
        num = va_arg(*arg, size_t);
        break;

        case LEN_T:
        /* TODO: ptrdiff_t.. */
        num = 0;
        break;
    }

    /* II) Translate Sign:  */
    /* -------------------- */
    if (num < 0) {
        sign = '-';
        num = -num;
    } else if (flags & F_FORCESIGN) {
        sign = '+';
    } else if (flags & F_SPACE) {
        sign = ' ';
    }

    /* III) Extract Digits:  */
    /* --------------------- */
    while(num) {
        buffer[bufi++] = num%10+'0';
        num /= 10;
    }

    /* IV) Calculate size of information:  */
    /* ----------------------------------- */
    prefix = sign > 0;
    digits = bufi > precision ? bufi : precision;
    if (digits + prefix < width) {
        if (flags & F_ZEROPAD) {
            /* zero padding. it is like precision */
            digits = width - prefix;
            before = 0;
            after  = 0;
        } else if (flags & F_LEFTJUSTIFY) {
            /* left justify, spaces leading... */
            before = 0;
            after = width - prefix - digits;
        } else {
            before = width - prefix - digits;
            after = 0;
        }
    } else {
        /* width is not enough.. */
        width  = prefix + digits;
        before = 0;
        after  = 0;
    }

    /* V) Print:  */
    /* ---------- */
    while(before--)
        PUTC_SUB(' ');

    while(prefix--)
        PUTC_SUB(sign);

    while(digits--)
        if (digits < bufi) {
            PUTC_SUB(buffer[--bufi]);
        } else {
            PUTC_SUB('0');
        }

    while(after--)
        PUTC_SUB(' ');

    /* VI) Return:  */
    /* ------------ */
    return 0; /* success */
}

/****************************************************************************/
/*                            parseUnsignedInt()                            */
/****************************************************************************/

int parseUnsignedInt(FILE *stream, char *str, int which, size_t n,
                     int n_is_valid, va_list *arg, size_t *count, char spec,
                     char flags, int width, int precision, int length) {

    char buffer[100];
    int bufi = 0;
    __extension__ long long num;
    char *baseStr = NULL;
    int base;
    char *dig;

    /* counters: */
    int before; /* spaces before prefix                   */
    int prefix; /* count of prefix characters (i.e, sign) */
    int digits; /* count digits that will be printed      */
    int after;  /* spaces after prefix                    */

    if (precision >= 0)
        /* precision is specified */
        flags &= ~F_ZEROPAD; /* ignore 0 flag. */
    else
        /* negative value of precision */
        precision = 1; /* default value */

    /* I) Detect Integer Type:  */
    /* ------------------------ */
    switch(length) {
        case LEN_NONE:
        num = va_arg(*arg, unsigned int);
        break;

        case LEN_HH:
        num = va_arg(*arg, unsigned char);
        break;

        case LEN_H:
        num = va_arg(*arg, unsigned short);
        break;

        case LEN_L:
        num = va_arg(*arg, unsigned long);
        break;

        case LEN_LL:
        case LEN_LCAP:
        num = __extension__ va_arg(*arg, unsigned long long);
        break;

        case LEN_J:
        /* TODO: intmax_t.. */
        num = 0;
        break;

        case LEN_Z:
        num = va_arg(*arg, size_t);
        break;

        case LEN_T:
        /* TODO: ptrdiff_t.. */
        num = 0;
        break;
    }

    /* II) Translate Base & Prefix:  */
    /* ----------------------------- */
    baseStr = "";
    prefix  = 0;

    if (spec == 'x') {
        base = 16;
        if (num != 0 && (flags & F_HASH)) {
            baseStr = "0x";
            prefix = 2;
        }
    } else if (spec == 'X') {
        base = 16;
        if (num != 0 && (flags & F_HASH)) {
            baseStr = "0X";
            prefix = 2;
        }
    } else if (spec == 'o') {
        base = 8;
    } else {
        base = 10;
    }

    /* III) Extract Digits:  */
    /* --------------------- */
    dig = "0123456789abcdef";
    if (spec == 'X')
        dig = "0123456789ABCDEF";

    while(num) {
        buffer[bufi++] = dig[num%base];
        num /= base;
    }

    /* IV) Calculate size of information:  */
    /* ----------------------------------- */
    digits = bufi > precision ? bufi : precision;

    /* according to the standard, '#' flag increases
     * precision (when used with %o) if and only if
     * necessary, to force the first digit to be zero.
     */
    if (base == 8 && (flags & F_HASH) && digits == bufi)
        digits++;

    if (digits + prefix < width) {
        if (flags & F_ZEROPAD) {
            /* zero padding. it is like precision = width - prefix_size */
            digits = width - prefix;
            before = 0;
            after  = 0;
        } else if (flags & F_LEFTJUSTIFY) {
            /* left justify, spaces leading... */
            before = 0;
            after = width - prefix - digits;
        } else {
            before = width - prefix - digits;
            after = 0;
        }
    } else {
        /* width is not enough.. */
        width  = prefix + digits;
        before = 0;
        after  = 0;
    }

    /* V) Print:  */
    /* ---------- */
    while(before--)
        PUTC_SUB(' ');

    while(prefix--)
        PUTC_SUB(*(baseStr++));

    while(digits--)
        if (digits < bufi) {
            PUTC_SUB(buffer[--bufi]);
        } else {
            PUTC_SUB('0');
        }

    while(after--)
        PUTC_SUB(' ');

    /* VI) Return:  */
    /* ------------ */
    return 0; /* success */
}

/****************************************************************************/
/*                               parseReal()                                */
/****************************************************************************/

int parseReal(FILE *stream, char *str, int which, size_t n,
              int n_is_valid, va_list *arg, size_t *count, char spec,
              char flags, int width, int precision, int length) {

#if 0
    long double num;
    int size;
    switch(length) {
        case LEN_NONE:
        case LEN_HH:
        case LEN_H:
        case LEN_L:
        case LEN_LL:
        case LEN_J:
        case LEN_Z:
        case LEN_T:
        num = va_arg(arg, double);
        size = sizeof(double);
        break;

        case LEN_LCAP:
        num = va_arg(arg, long double);
        size = sizeof(long double);
        break;
    }
    /* now parse the double */
    break;
#endif
}

/****************************************************************************/
/*                               parseChar()                                */
/****************************************************************************/

int parseChar(FILE *stream, char *str, int which, size_t n,
              int n_is_valid, va_list *arg, size_t *count, char spec,
              char flags, int width, int precision, int length) {

    char c;

    int before;
    int after;

    /* I) Detect Integer Type:  */
    /* ------------------------ */
    switch(length) {
        case LEN_NONE:
        case LEN_HH:
        case LEN_H:
        case LEN_LL:
        case LEN_J:
        case LEN_Z:
        case LEN_T:
        case LEN_LCAP:
        c = va_arg(*arg,int);
        break;

        case LEN_L:
        /*TODO: wide character stuff. */
        c = 0;
        break;
    }

    /* II) Calculate size of information:  */
    /* ----------------------------------- */
    if (width > 1) {
        if (flags & F_LEFTJUSTIFY) {
            before = 0;
            after = width-1;
        } else {
            after  = 0;
            before = width-1;
        }
    } else {
        before = 0;
        after  = 0;
    }

    /* III) Print the character:  */
    /* -------------------------- */
    while(before--)
        PUTC_SUB(' ');

    PUTC_SUB(c);

    while(after--)
        PUTC_SUB(' ');

    /* IV) Return:  */
    /* ------------ */
    return 0; /* success */
}

/****************************************************************************/
/*                              parseString()                               */
/****************************************************************************/

int parseString(FILE *stream, char *str, int which, size_t n,
                int n_is_valid, va_list *arg, size_t *count, char spec,
                char flags, int width, int precision, int length) {

    char *s;

    int before;
    int len;
    int after;

    /* I) Detect Integer Type:  */
    /* ------------------------ */
    switch(length) {
        case LEN_NONE:
        case LEN_HH:
        case LEN_H:
        case LEN_LL:
        case LEN_J:
        case LEN_Z:
        case LEN_T:
        case LEN_LCAP:
        s = va_arg(*arg, char *);
        break;

        case LEN_L:
        /*TODO: wide character stuff. */
        s = "";
        break;
    }


    /* II) Calculate size of information:  */
    /* ----------------------------------- */
    len = strlen(s);
    if (precision >= 0 && len > precision)
        len = precision;

    if (width > len) {
        if (flags & F_LEFTJUSTIFY) {
            before = 0;
            after  = width-len;
        } else {
            before = width-len;
            after  = 0;
        }
    } else {
        before = 0;
        after  = 0;
    }

    /* III) Print the character:  */
    /* -------------------------- */
    while(before--)
        PUTC_SUB(' ');

    while(len--)
        PUTC_SUB(*(s++));

    while(after--)
        PUTC_SUB(' ');

    /* IV) Return:  */
    /* ------------ */
    return 0; /* success */
}

/****************************************************************************/
/*                              parsePointer()                              */
/****************************************************************************/

int parsePointer(FILE *stream, char *str, int which, size_t n,
                 int n_is_valid, va_list *arg, size_t *count, char spec,
                 char flags, int width, int precision, int length) {

    void *p = va_arg(*arg, void *);
    va_list t;
    /* FIXME: pointers are not always int. */
    __extension__ unsigned long long pll;

    if (p == NULL) {
        char *s = "(null)";
        int len = strlen(s);

        int before = 0;
        int after  = 0;

        if (width > len) {
            if (flags & F_LEFTJUSTIFY)
                after = width-len;
            else
                before = width-len;
        }

        while(before--)
            PUTC_SUB(' ');

        while(len--)
            PUTC_SUB(*(s++));

        while(after--)
            PUTC_SUB(' ');

        return 0;
    }

    t = &pll;
    flags |= F_HASH;

    if (sizeof(void *) == sizeof(long)) {
        /* 32 bit */
        pll = (unsigned long) p;
        length = LEN_L;
    } else {
        /* 64 bit */
        pll = __extension__ (unsigned long long) p;
        length = LEN_LL;
    }

    return parseUnsignedInt(stream, str, which, n, n_is_valid,
                            &t, count, 'x', flags, width,
                            precision, length);
}

/****************************************************************************/
/*                              parseCount()                                */
/****************************************************************************/

int parseCount(FILE *stream, char *str, int which, size_t n,
               int n_is_valid, va_list *arg, size_t *count, char spec,
               char flags, int width, int precision, int length) {

    switch(length) {
        case LEN_NONE:
        *((int *) va_arg(*arg, int *)) = *count;
        break;

        case LEN_HH:
        *((char *) va_arg(*arg, char *)) = *count;
        break;

        case LEN_H:
        *((short *) va_arg(*arg, short *)) = *count;
        break;

        case LEN_L:
        *((long *) va_arg(*arg, long *)) = *count;
        break;

        case LEN_LL:
        case LEN_LCAP:
        __extension__  *((long long *) va_arg(*arg, long long *)) = *count;
        break;

        case LEN_J:
        /* TODO: intmax_t.. */

        break;

        case LEN_Z:
        *((size_t *) va_arg(*arg, size_t *)) = *count;
        break;

        case LEN_T:
        /* TODO: ptrdiff_t.. */

        break;
    }

    return 0;
}

/****************************************************************************/
/*                              __vsfprintf()                               */
/****************************************************************************/

int __vsfprintf(FILE *stream, char *str, int which, size_t n,
                int n_is_valid, const char *fmt, va_list arg) {

    size_t count = 0; /* count of written characters. */

    /* specifier settings:  */
    /* -------------------- */
    int i = 0; /* stage.. */
    char flags;
    int width;
    int precision;
    int length;

    while(*fmt) {
        if (i == 0) {
            /* a '%' or a normal character expected. */
            if (*fmt != '%') {
                /* normal char. */
                PUTC(*fmt);
            } else {
                /* oh a specifier */
                i = 1;
                flags = 0;
                width = 0;
                precision = -1;
                length = LEN_NONE;
            }
            fmt++;
        } else if (i == 1) {
            /* flag expected */
            switch(*fmt) {
                case '-':
                flags |= F_LEFTJUSTIFY;
                fmt++;
                break;

                case '+':
                flags |= F_FORCESIGN;
                fmt++;
                break;

                case ' ':
                flags |= F_SPACE;
                fmt++;
                break;

                case '#':
                flags |= F_HASH;
                fmt++;
                break;

                case '0':
                flags |= F_ZEROPAD;
                fmt++;
                break;

                default:
                i++;
                break;
            }
        } else if (i == 2) {
            /* width expected */
            if (*fmt == '*' && width == 0) {
                width = va_arg(arg, int);
                fmt++;
            } else if (*fmt >= '0' && *fmt <= '9') {
                width *= 10;
                width += (*fmt - '0');
                fmt++;
            } else {
                i++;
            }
        } else if (i == 3) {
            /* dot expected */
            if (*fmt == '.' && *(fmt+1) == '*') {
                /* precision provided as an arg. */
                precision = va_arg(arg, int);
                fmt+=2;
                i += 2;
            } else if (*fmt == '.') {
                precision = 0;
                fmt++;
                i++; /* precision is following.. */
            } else {
                i+=2; /* no precision is following.. */
            }
        } else if (i == 4) {
            /* precision expected */
            if (*fmt >= '0' && *fmt <= '9') {
                precision *= 10;
                precision += (*fmt - '0');
                fmt++;
            } else {
                i++;
            }
        } else if (i == 5) {
            /* length expected */
            if (*fmt == 'h' && *(fmt+1) == 'h') {
                length = LEN_HH;
                fmt += 2;
            } else if (*fmt == 'h') {
                length = LEN_H;
                fmt++;
            } else if (*fmt == 'l' && *(fmt+1) == 'l') {
                length = LEN_LL;
                fmt += 2;
            } else if (*fmt == 'l') {
                length = LEN_L;
                fmt++;
            } else if (*fmt == 'j') {
                length = LEN_J;
                fmt++;
            } else if (*fmt == 'z') {
                length = LEN_Z;
                fmt++;
            } else if (*fmt == 't') {
                length = LEN_T;
                fmt++;
            } else if (*fmt == 'L') {
                length = LEN_LCAP;
                fmt++;
            }
            i++;
        } else if (i == 6) {
            /* THE SPECIFIER ITSELF */

            /* Do some corrections:  */
            /* --------------------- */
            if (width < 0) {
                /* negative value of width. */
                width = -width;
                flags |= F_LEFTJUSTIFY; /* set '-' flag. */
            }

            if (flags & F_LEFTJUSTIFY)
                /* '-' flag is specified. */
                flags &= ~F_ZEROPAD; /* ignore 0 flag. */

            if (flags & F_FORCESIGN)
                /* '+' flag is specified. */
                flags &= ~F_SPACE; /* ignore ' ' flag. */

            /* Test the specifier:  */
            /* -------------------- */
            switch(*fmt) {
                case 'd':
                case 'i':
                if (parseSignedInt(stream, str, which, n,
                                   n_is_valid, &arg, &count,
                                   *fmt, flags, width,
                                   precision, length))
                    return count; /* error. */
                break;

                case 'u':
                case 'o':
                case 'x':
                case 'X':
                if (parseUnsignedInt(stream, str, which, n,
                                     n_is_valid, &arg, &count,
                                     *fmt, flags, width,
                                     precision, length))
                    return count; /* error. */
                break;

                case 'f':
                case 'F':
                case 'e':
                case 'E':
                case 'g':
                case 'G':
                case 'a':
                case 'A':
                if (parseReal(stream, str, which, n,
                              n_is_valid, &arg, &count,
                              *fmt, flags, width,
                              precision, length))
                    return count; /* error. */
                break;

                case 'c':
                if (parseChar(stream, str, which, n,
                              n_is_valid, &arg, &count,
                              *fmt, flags, width,
                              precision, length))
                    return count; /* error. */
                break;

                case 's':
                if (parseString(stream, str, which, n,
                                n_is_valid, &arg, &count,
                                *fmt, flags, width,
                                precision, length))
                    return count; /* error. */
                break;

                case 'p':
                if (parsePointer(stream, str, which, n,
                                 n_is_valid, &arg, &count,
                                 *fmt, flags, width,
                                 precision, length))
                    return count; /* error. */
                break;

                case 'n':
                if (parseCount(stream, str, which, n,
                               n_is_valid, &arg, &count,
                               *fmt, flags, width,
                               precision, length))
                    return count; /* error. */
                break;

                case '%':
                PUTC('%');
                break;

                default:
                /* unknown format specifier, skip.. */
                break;
            }
            fmt++;
            i = 0; /* reset. */
        }

    }
    return count;
}

/****************************************************************************/
/*                                scanInt()                                 */
/****************************************************************************/

int scanInt(FILE *stream, char *str, int which, va_list *arg,
            size_t *count, size_t *items, char **fmt, int ignore,
            int width, int length) {

    /* statistics: */
    int t;
    int read   = 0; /* count of characters been read. */
    int digits = 0; /* count of digits.               */

    /* number: */
    char sign = 0;
    __extension__ unsigned long long num = 0;
    int base = -1;

    /* the specifier: */
    char spec = **fmt;
    (*fmt)++;

    /* determine base: */
    if (spec == 'o') {
        base = 8;
    } else if (spec == 'd' || spec == 'u') {
        base = 10;
    } else if (spec == 'x' || spec == 'X' || spec == 'p') {
        base = 16;
    }

    /* skip spaces: */
    while (1) {
        t = GETC_SUB();
        if (!isspace(t)) break;
    }
    UNGETC(t);

    /* read sign: */
    if ((!width) || read < width) {
        t = GETC_SUB();
        if (t == '+' || t == '-') {
            sign = t;
            read++;
        } else {
            UNGETC_SUB(t);
        }
    }

    /* read number: */
    while((!width) || read < width) {
        t = GETC_SUB();

        /* Determine Base: */
        if (digits == 0 && spec == 'i') {
            if (t == '0') {
                /* first digit is zero */
                base = 8;
            } else if (t >= '1' && t <= '9') {
                /* must be decimal number. */
                base = 10;
            } else {
                /* unkown character */
                UNGETC_SUB(t);
                break;
            }
        }

        if ((t == 'x' || t == 'X') && digits == 1 && num == 0) {
            /* the 0x prefix */
            if (spec == 'd' || spec == 'u' || spec == 'o') {
                /* x is unknown character */
                UNGETC_SUB(t);
                break;
            } else {
                base = 16;
                read++;
                continue;
            }
        }

        /* make sure digits are inside boundary:  */
        /* -------------------------------------- */
        if (base == 8 && (t < '0' || t > '7')) {
            /* not a digit */
            UNGETC_SUB(t);
            break;
        }

        if (base == 10 && (t < '0' || t > '9')) {
            /* not a digit */
            UNGETC_SUB(t);
            break;
        }

        if (base == 16 && (!((t >= '0' && t <= '9') ||
                          (t >= 'A' && t <= 'F')    ||
                          (t >= 'a' && t <= 'f')))) {
            /* not a digit */
            UNGETC_SUB(t);
            break;
        }

        /* Translate digits:  */
        /* ------------------ */
        if (t >= '0' && t <= '9')
            num = num*base + (t-'0');
        else if (t >= 'A' && t <= 'F')
            num = num*base + (t-'A'+0xA);
        else
            num = num*base + (t-'a'+0xA);
        read++;
        digits++;
    }

    /* validate number: */
    if (digits == 0) {
        /* read error... */
        return 1;
    }

    /* translate sign: */
    if (sign == '-')
        num = -num;

    /* done? */
    if (ignore) {
        /* done... */
        return 0;
    }

    switch(length) {
        case LEN_NONE:
        *((int *) va_arg(*arg, int *)) = num;
        break;

        case LEN_HH:
        *((char *) va_arg(*arg, char *)) = num;
        break;

        case LEN_H:
        *((short *) va_arg(*arg, short *)) = num;
        break;

        case LEN_L:
        *((long *) va_arg(*arg, long *)) = num;
        break;

        case LEN_LL:
        case LEN_LCAP:
        __extension__ *((long long *) va_arg(*arg, long long *)) = num;
        break;

        case LEN_J:
        /* TODO: intmax_t.. */

        break;

        case LEN_Z:
        *((size_t *) va_arg(*arg, size_t *)) = num;
        break;

        case LEN_T:
        /* TODO: ptrdiff_t.. */

        break;
    }
    (*items)++;

    return 0;

}

/****************************************************************************/
/*                               scanChar()                                 */
/****************************************************************************/

int scanChar(FILE *stream, char *str, int which, va_list *arg,
             size_t *count, size_t *items, char **fmt, int ignore,
             int width, int length) {

    int t;

    /* the specifier: */
    char spec = **fmt;
    (*fmt)++;

    t = GETC_SUB();
    if (t == EOF)
        return 1; /* error; */

    /* done? */
    if (ignore) {
        /* done... */
        return 0;
    }

    /* store t: */
    switch(length) {
        case LEN_NONE:
        case LEN_HH:
        case LEN_H:
        case LEN_LL:
        case LEN_J:
        case LEN_Z:
        case LEN_T:
        case LEN_LCAP:
        *((char *) va_arg(*arg,char *)) = t;
        break;

        case LEN_L:
        /*TODO: */
        /* wide character stuff. */
        break;
    }
    (*items)++;

    return 0;
}

/****************************************************************************/
/*                              scanString()                                */
/****************************************************************************/

int scanString(FILE *stream, char *str, int which, va_list *arg,
               size_t *count, size_t *items, char **fmt, int ignore,
               int width, int length) {

    int t;
    int read = 0;
    char *s = NULL;

    /* the specifier: */
    char spec = **fmt;
    (*fmt)++;

    /* get string: */
    if (!ignore) {
        switch(length) {
            case LEN_NONE:
            case LEN_HH:
            case LEN_H:
            case LEN_LL:
            case LEN_J:
            case LEN_Z:
            case LEN_T:
            case LEN_LCAP:
            s = va_arg(*arg, char *);
            break;

            case LEN_L:
            /*TODO: */
            /* wide character stuff. */
            break;
        }

    }

    /* skip spaces: */
    while (1) {
        t = GETC_SUB();
        if (!isspace(t)) break;
    }
    UNGETC_SUB(t);

    /* read string: */
    while ((!width) || read < width) {
        t = GETC_SUB();

        if (t == EOF || isspace(t)) {
            UNGETC_SUB(t);
            break;
        }

        if (!ignore)
            s[read++] = t;
    }

    if (read == 0)
        return 2; /* error */

    s[read] = '\0';
    return 0;

}

/****************************************************************************/
/*                              __vsfscanf()                                */
/****************************************************************************/

int __vsfscanf(FILE *stream, char *str, int which, char *fmt, va_list arg) {

    size_t count = 0; /* how much i've read till now. */
    size_t items = 0;

    /* specifier settings:  */
    /* -------------------- */
    int i = 0; /* stage.. */
    int ignore;
    int width;
    int length;

    while(*fmt) {
        if (i == 0) {
            if (*fmt == '%') {
                /* oh a specifier */
                i = 1;
                ignore = 0;
                width = 0;
                length = LEN_NONE;
            } else if (isspace(*fmt)) {
                /* read spaces from stream. */
                int t;
                while (1) {
                    t = GETC();
                    if (!isspace(t)) break;
                }
                UNGETC(t);
            } else {
                /* normal char. must match first read from stream. */
                int t = GETC();
                if (t != *fmt) {
                    UNGETC(t);
                    /* error not matching. */
                    break;
                };
                count++;
            }
            fmt++;
        } else if (i == 1) {
            if (*fmt == '*') {
                ignore = 1;
                fmt++;
            }
            i++; /* go to next step */
        } else if (i == 2) {
            if (*fmt >= '0' && *fmt <= '9') {
                width = width*10 + (*fmt - '0');
                fmt++;
            } else {
                i++; /* go to next step */
            }
        } else if (i == 3) {
            if (*fmt == 'h' && *(fmt+1) == 'h') {
                length = LEN_HH;
                fmt += 2;
            } else if (*fmt == 'h') {
                length = LEN_H;
                fmt++;
            } else if (*fmt == 'l' && *(fmt+1) == 'l') {
                length = LEN_LL;
                fmt += 2;
            } else if (*fmt == 'l') {
                length = LEN_L;
                fmt++;
            } else if (*fmt == 'j') {
                length = LEN_J;
                fmt++;
            } else if (*fmt == 'z') {
                length = LEN_Z;
                fmt++;
            } else if (*fmt == 't') {
                length = LEN_T;
                fmt++;
            } else if (*fmt == 'L') {
                length = LEN_LCAP;
                fmt++;
            }
            i++;
        } else if (i == 4) {
            /* the specifier. */
            int brk = 0;
            switch(*fmt) {
                case 'i':
                case 'd':
                case 'u':
                case 'o':
                case 'x':
                case 'X':
                case 'p':
                if (scanInt(stream, str, which, &arg,
                            &count, &items, &fmt, ignore,
                            width, length))
                    brk = 1;
                break;

                case 'c':
                if (scanChar(stream, str, which, &arg,
                             &count, &items, &fmt, ignore,
                             width, length))
                    brk = 1;
                break;

                case 's':
                if (scanString(stream, str, which, &arg,
                               &count, &items, &fmt, ignore,
                               width, length))
                    brk = 1;
                break;

                default:
                break; /*ignore */
            }
        }
    }
    return count ? items : EOF;

}

/****************************************************************************/
/*                                fprintf()                                 */
/****************************************************************************/

int fprintf(FILE *stream, const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    return __vsfprintf(stream, NULL, 0, 0, 0, format, arg);
}

/****************************************************************************/
/*                                 printf()                                 */
/****************************************************************************/

int printf(const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    return __vsfprintf(stdout, NULL, 0, 0, 0, format, arg);
}

/****************************************************************************/
/*                                fscanf()                                  */
/****************************************************************************/

int fscanf(FILE *stream, const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    return __vsfscanf(stream, NULL, 0, (char *) format, arg);
}

/****************************************************************************/
/*                                 scanf()                                  */
/****************************************************************************/

int scanf(const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    return __vsfscanf(stdin, NULL, 0, (char *) format, arg);
}
