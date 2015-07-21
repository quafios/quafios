/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Virtual Console Device Driver.                   | |
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
#include <lib/linkedlist.h>
#include <sys/error.h>
#include <sys/printk.h>
#include <sys/class.h>
#include <sys/resource.h>
#include <sys/device.h>
#include <sys/scheduler.h>
#include <tty/vtty.h>
#include <tty/pstty.h>

/* Prototypes: */
uint32_t vtty_probe(device_t *, void *);
uint32_t vtty_read (device_t *, uint64_t, uint32_t, char *);
uint32_t vtty_write(device_t *, uint64_t, uint32_t, char *);
uint32_t vtty_ioctl(device_t *, uint32_t, void *);
uint32_t vtty_irq  (device_t *, uint32_t);

/* legacy variables */
extern uint32_t legacy_lfb_enabled;

/* options: */
int32_t echo = 1;
int32_t bufbyline = 1;

/* buffer: */
char inbuf[4096]; /* input buffer; */
int32_t buffront = 0;
int32_t bufback = 0;
int32_t buflines = 0; /* count of lines buffered... */

static int32_t blocked_pid = -1;

/* Classes supported: */
static class_t classes[] = {
    {BUS_GENESIS, BASE_GENESIS_TTY, SUB_GENESIS_TTY_VIRTUAL, IF_ANY}
};

/* driver_t structure that identifies this driver: */
driver_t vtty_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "vtty",
    /* probe:     */ vtty_probe,
    /* read:      */ vtty_read,
    /* write:     */ vtty_write,
    /* ioctl:     */ vtty_ioctl,
    /* irq:       */ vtty_irq
};

static void print_char(char c) {
    legacy_video_putc(c);
}

static void change_attr(char attr) {
    legacy_video_attr(attr);
}

static void press(char c) {
    int32_t count;
    if (c == '\b' && echo) {
        /* special treating... */
        count = 1;

        /* if buffer is empty... cancel */
        if (buffront == bufback)
            return;

        /* if last written character was '\n', cancel */
        if (inbuf[(bufback-1+sizeof(inbuf))%sizeof(inbuf)]=='\n')
            return;

        /* if last written character was '\t', backspace n times
         * where n is co-ordinate related.
         */
        if (inbuf[(bufback-1+sizeof(inbuf))%sizeof(inbuf)]=='\t')
            count = 8;

        /* do it! */
        while(count--) {
            bufback = (bufback-1+sizeof(inbuf))%sizeof(inbuf);
            print_char(c);
            print_char(' ');
            print_char(c);
        }

        return;
    }

    if ((bufback + 1) % sizeof(inbuf) == buffront) {
        /* buffer is full! */
        return;
    }

    if ((bufback + 2) % sizeof(inbuf) == buffront) {
        /* only one place is available in the buffer.
         * this can be used only for '\n';
         * any other character will be discarded.
         */
        if (c != '\n')
            return;
    }

    /* insert the character into buffer! */
    if (echo)
        print_char(c);
    inbuf[bufback] = c;
    bufback = (bufback + 1) % sizeof(inbuf);
    if (bufbyline && c == '\n')
        buflines++;

}

/* ================================================================= */
/*                            Interface                              */
/* ================================================================= */

uint32_t vtty_probe(device_t *dev, void *config) {
    /* tell the system that i am the system console: */
    system_console = (void *) dev;
    devfs_reg("console", dev->devid);
    return ESUCCESS;
}

uint32_t vtty_read(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    int32_t count = size;
    while(count--) {
        if (bufbyline) {
            while(!buflines) {
                blocked_pid = curproc->pid;
                block();
            }
        } else {
            while (buffront == bufback) {
                /* buffer is empty. */
                blocked_pid = curproc->pid;
                block();
            }
        }
        *(buff++) = inbuf[buffront];
        if (bufbyline && inbuf[buffront] == '\n')
            buflines--;
        buffront = (buffront + 1) % sizeof(inbuf);
    }
    return size;
}

uint32_t vtty_write(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    int i = 0;
    while(i < size)
        print_char(buff[i++]);
    return size;
}

uint32_t vtty_ioctl(device_t *dev, uint32_t cmd, void *data) {

    device_t *t;
    class_t cls;
    reslist_t reslist = {0, NULL};
    pstty_init_t pstty_config;

    switch (cmd) {
        case TTY_PRESS:
            press(*((uint8_t *) data));
            if (blocked_pid != -1) {
                unblock(blocked_pid);
                blocked_pid = -1;
            }
            break;
        case TTY_ATTR:
            legacy_video_attr(*((uint8_t *) data));
            break;
        case TTY_GETCURSOR:
            legacy_get_cursor(((char *)data)+0,
                              ((char *)data)+1);
            break;
        case TTY_SETATTRATOFF:
            legacy_set_attr_at_off(((char *)data)[0],
                                   ((char *)data)[1],
                                   ((char *)data)[2]);
            break;
        case TTY_SETCHARATOFF:
            legacy_set_char_at_off(((char *)data)[0],
                                   ((char *)data)[1],
                                   ((char *)data)[2]);
            break;
        case TTY_SETCURSOR:
            legacy_set_cursor(((char *)data)[0],
                              ((char *)data)[1]);
            break;
        case TTY_NOECHO:
            echo = 0;
            break;
        case TTY_SETECHO:
            echo = 1;
            break;
        case TTY_NOBUF:
            buffront = 0;
            bufback = 0;
            buflines = 0;
            bufbyline = 0;
            break;
        case TTY_SETBUF:
            buffront = 0;
            bufback = 0;
            buflines = 0;
            bufbyline = 1;
            break;
        case TTY_DISABLE:
            legacy_lfb_enabled = 0;
            break;

        case TTY_ENABLE:
            legacy_lfb_enabled = 1;
            legacy_redraw();
            break;

        case TTY_FORK:
            cls.bus  = BUS_GENESIS;
            cls.base = BASE_GENESIS_TTY;
            cls.sub  = SUB_GENESIS_TTY_PSEUDO;
            pstty_config.pid = curproc->pid;
            pstty_config.prefix = ((tty_fork_t *) data)->prefix;
            dev_add(&t, dev->parent_bus, cls, reslist, &pstty_config);
            ((tty_fork_t *) data)->devid = t->devid;
            break;

        default:
            break;
    }
    return ESUCCESS;
}

uint32_t vtty_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
