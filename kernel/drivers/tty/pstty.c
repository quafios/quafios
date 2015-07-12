/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> Pseudo TTY Device Driver.                        | |
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
#include <lib/linkedlist.h>
#include <sys/error.h>
#include <sys/printk.h>
#include <sys/class.h>
#include <sys/resource.h>
#include <sys/device.h>
#include <sys/mm.h>
#include <sys/ipc.h>
#include <tty/vtty.h>
#include <tty/pstty.h>

/* Prototypes: */
uint32_t pstty_probe(device_t *, void *);
uint32_t pstty_read (device_t *, uint64_t, uint32_t, char *);
uint32_t pstty_write(device_t *, uint64_t, uint32_t, char *);
uint32_t pstty_ioctl(device_t *, uint32_t, void *);
uint32_t pstty_irq  (device_t *, uint32_t);

#define BUFSIZE         4096

typedef struct info {

    /* options: */
    int32_t echo;
    int32_t bufbyline;

    /* buffer: */
    char inbuf[4096]; /* input buffer; */
    int32_t buffront;
    int32_t bufback;
    int32_t buflines; /* count of lines buffered... */

    /* a copy of x and y locations */
    int32_t x;
    int32_t y;

    /* information about the owner process */
    int pid;
    char prefix;

    /* 1 when cursor changes */
    int cursor_set;

    /* blocked process */
    int32_t blocked_pid;

} info_t;

/* Classes supported: */
static class_t classes[] = {
    {BUS_GENESIS, BASE_GENESIS_TTY, SUB_GENESIS_TTY_PSEUDO, IF_ANY}
};

/* driver_t structure that identifies this driver: */
driver_t pstty_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "pstty",
    /* probe:     */ pstty_probe,
    /* read:      */ pstty_read,
    /* write:     */ pstty_write,
    /* ioctl:     */ pstty_ioctl,
    /* irq:       */ pstty_irq
};

static void putc(info_t *info, char c) {
    /* send a packet to the owner process */
    pstty_packet_t packet = {0};
    msg_t msg;

    /* initialize packet */
    packet.prefix = info->prefix;
    packet.cmd    = PSTTY_PUTC;
    packet.chr    = c;

    /* initialize msg */
    msg.size = sizeof(packet);
    msg.buf  = &packet;

    /* do the send! */
    send(info->pid, &msg);
}

static void change_attr(info_t *info, char attr) {
    /* send a packet to the owner process */
    pstty_packet_t packet = {0};
    msg_t msg;

    /* initialize packet */
    packet.prefix = info->prefix;
    packet.cmd    = PSTTY_CHANGE_ATTR;
    packet.attr   = attr;

    /* initialize msg */
    msg.size = sizeof(packet);
    msg.buf  = &packet;

    /* do the send! */
    send(info->pid, &msg);
}

static void get_cursor(info_t *info, char *x, char *y) {
    /* send a packet to the owner process */
    pstty_packet_t packet = {0};
    msg_t msg;

    /* initialize packet */
    packet.prefix = info->prefix;
    packet.cmd    = PSTTY_GET_CURSOR;

    /* initialize msg */
    msg.size = sizeof(packet);
    msg.buf  = &packet;

    /* do the send! */
    send(info->pid, &msg);

    /* wait until we receive the reply */
    while (!info->cursor_set);

    /* return */
    info->cursor_set = 0;
    *x = info->x;
    *y = info->y;
}

static void set_attr_at_off(info_t *info, char x, char y, char attr) {
    /* send a packet to the owner process */
    pstty_packet_t packet = {0};
    msg_t msg;

    /* initialize packet */
    packet.prefix = info->prefix;
    packet.cmd    = PSTTY_SET_ATTR_AT_OFF;
    packet.attr   = attr;
    packet.x      = x;
    packet.y      = y;

    /* initialize msg */
    msg.size = sizeof(packet);
    msg.buf  = &packet;

    /* do the send! */
    send(info->pid, &msg);
}

static void set_char_at_off(info_t *info, char x, char y, char c) {
    /* send a packet to the owner process */
    pstty_packet_t packet = {0};
    msg_t msg;

    /* initialize packet */
    packet.prefix = info->prefix;
    packet.cmd    = PSTTY_SET_CHAR_AT_OFF;
    packet.chr    = c;
    packet.x      = x;
    packet.y      = y;

    /* initialize msg */
    msg.size = sizeof(packet);
    msg.buf  = &packet;

    /* do the send! */
    send(info->pid, &msg);
}

static void set_cursor(info_t *info, char x, char y) {

    pstty_packet_t packet = {0};
    msg_t msg;
    info->x = x;
    info->y = y;
    info->cursor_set = 1;

    /* initialize packet */
    packet.prefix = info->prefix;
    packet.cmd    = PSTTY_SET_CURSOR;
    packet.x      = x;
    packet.y      = y;

    /* initialize msg */
    msg.size = sizeof(packet);
    msg.buf  = &packet;

    /* do the send! */
    send(info->pid, &msg);
}

static void press(info_t *info, char c) {
    int32_t count;
    if (c == '\b' && info->echo) {
        /* special treating... */
        count = 1;

        /* if buffer is empty... cancel */
        if (info->buffront == info->bufback)
            return;

        /* if last written character was '\n', cancel */
        if (info->inbuf[(info->bufback-1+BUFSIZE)%BUFSIZE]=='\n')
            return;

        /* if last written character was '\t', backspace n times
         * where n is co-ordinate related.
         */
        if (info->inbuf[(info->bufback-1+BUFSIZE)%BUFSIZE]=='\t')
            count = 8;

        /* do it! */
        while(count--) {
            info->bufback = (info->bufback-1+BUFSIZE)%BUFSIZE;
            putc(info, c);
            putc(info, ' ');
            putc(info, c);
        }

        return;
    }

    if ((info->bufback + 1) % BUFSIZE == info->buffront) {
        /* buffer is full! */
        return;
    }

    if ((info->bufback + 2) % BUFSIZE == info->buffront) {
        /* only one place is available in the buffer.
         * this can be used only for '\n';
         * any other character will be discarded.
         */
        if (c != '\n')
            return;
    }

    /* insert the character into buffer! */
    if (info->echo)
        putc(info, c);
    info->inbuf[info->bufback] = c;
    info->bufback = (info->bufback + 1) % BUFSIZE;
    if (info->bufbyline && c == '\n')
        info->buflines++;

}

/* ================================================================= */
/*                            Interface                              */
/* ================================================================= */

uint32_t pstty_probe(device_t *dev, void *config) {

    /* allocate info_t structure */
    info_t *info = kmalloc(sizeof(info_t));
    dev->drvreg = (uint32_t) info;
    if (info == NULL)
        return ENOMEM;

    /* initialize info_t */
    info->echo       = 1;
    info->bufbyline  = 1;
    info->buffront   = 0;
    info->bufback    = 0;
    info->buflines   = 0;
    info->pid        = ((pstty_init_t *) config)->pid;
    info->prefix     = ((pstty_init_t *) config)->prefix;
    info->cursor_set = 0;
    info->blocked_pid = -1;

    /* done */
    return ESUCCESS;
}

uint32_t pstty_read(device_t *dev, uint64_t off, uint32_t size, char *buff) {

    int32_t count = size;
    int i = 0;
    extern uint32_t flag;

    /* get info_t structure: */
    info_t *info = (info_t *) dev->drvreg;
    if (info == NULL)
            return ENOMEM; /* i am sorry :D */

    /* loop and read from the buffer */
    while(count--) {
        if (info->bufbyline) {
            while(!info->buflines) {
                /* block the process */
                info->blocked_pid = getpid();
                block();
            }
        } else {
            while(info->buffront == info->bufback) {
                /* buffer is empty. */
                info->blocked_pid = getpid();
                block();
            }
        }
        *(buff++) = info->inbuf[info->buffront];
        if (info->bufbyline && info->inbuf[info->buffront] == '\n')
            info->buflines--;
        info->buffront = (info->buffront + 1) % BUFSIZE;
    }

    /* return the size of the read */
    return size;
}

uint32_t pstty_write(device_t *dev, uint64_t off, uint32_t size, char *buff) {

    int i = 0;

    /* get info_t structure: */
    info_t *info = (info_t *) dev->drvreg;
    if (info == NULL)
            return ENOMEM; /* i am sorry :D */

    /* write to the screen */
    while(i < size)
        putc(info, buff[i++]);

    /* done */
    return size;

}

uint32_t pstty_ioctl(device_t *dev, uint32_t cmd, void *data) {

    /* get info_t structure: */
    info_t *info = (info_t *) dev->drvreg;
    if (info == NULL)
            return ENOMEM; /* i am sorry :D */

    /* process the request */
    switch (cmd) {
        case TTY_PRESS:
            /* the keyboard calls this when a key is pressed */
            press(info, *((uint8_t *) data));
            /* unblock the waiting process */
            if (info->blocked_pid != -1) {
                unblock(info->blocked_pid);
                info->blocked_pid = -1;
            }
            break;
        case TTY_ATTR:
            change_attr(info, *((uint8_t *) data));
            break;
        case TTY_GETCURSOR:
            get_cursor(info, ((char *)data)+0,
                             ((char *)data)+1);
            break;
        case TTY_SETATTRATOFF:
            set_attr_at_off(info, ((char *)data)[0],
                                  ((char *)data)[1],
                                  ((char *)data)[2]);
            break;
        case TTY_SETCHARATOFF:
            set_char_at_off(info, ((char *)data)[0],
                                  ((char *)data)[1],
                                  ((char *)data)[2]);
            break;
        case TTY_SETCURSOR:
            set_cursor(info, ((char *)data)[0],
                             ((char *)data)[1]);
            break;
        case TTY_NOECHO:
            info->echo = 0;
            break;
        case TTY_SETECHO:
            info->echo = 1;
            break;
        case TTY_NOBUF:
            info->buffront = 0;
            info->bufback = 0;
            info->buflines = 0;
            info->bufbyline = 0;
            break;
        case TTY_SETBUF:
            info->buffront = 0;
            info->bufback = 0;
            info->buflines = 0;
            info->bufbyline = 1;
            break;
        case TTY_DISABLE:
            /* do nothing */
            break;

        case TTY_ENABLE:
            /* do nothing */
            break;

        default:
            break;
    }
    return ESUCCESS;
}

uint32_t pstty_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
