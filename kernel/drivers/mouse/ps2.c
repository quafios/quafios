/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> PS/2 Mouse Device Driver.                        | |
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
#include <sys/mm.h>
#include <sys/class.h>
#include <sys/resource.h>
#include <sys/device.h>
#include <serio/8042kbc.h>
#include <tty/vtty.h>
#include <mouse/generic.h>
#include <sys/bootinfo.h>
#include <sys/scheduler.h>

/* Prototypes: */
uint32_t ps2mouse_probe(device_t *, void *);
uint32_t ps2mouse_read (device_t *, uint64_t, uint32_t, char *);
uint32_t ps2mouse_write(device_t *, uint64_t, uint32_t, char *);
uint32_t ps2mouse_ioctl(device_t *, uint32_t, void *);
uint32_t ps2mouse_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_SERIO, BASE_i8042_ATKBC, SUB_i8042_ATKBC_PS2MOUSE, IF_ANY}
};

int32_t mouse_x   = 0;
int32_t mouse_y   = 0;
int32_t left_btn  = 0;
int32_t mid_btn   = 0;
int32_t right_btn = 0;
int32_t x_sign    = 0;
int32_t y_sign    = 0;
int32_t x_of      = 0;
int32_t y_of      = 0;
int32_t mouse_counter = 0; /* packet counter */
static int32_t listener_pid = -1;
static uint8_t listener_prefix = -1;
static bootinfo_t *bootinfo = (bootinfo_t *) 0x10000;
extern uint32_t legacy_lfb_enabled;

/* driver_t structure that identifies this driver: */
driver_t ps2_mouse = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "ps2_mouse",
    /* probe:     */ ps2mouse_probe,
    /* read:      */ ps2mouse_read,
    /* write:     */ ps2mouse_write,
    /* ioctl:     */ ps2mouse_ioctl,
    /* irq:       */ ps2mouse_irq
};

/* ================================================================= */
/*                             Commands                              */
/* ================================================================= */

static uint8_t read_packet(device_t *dev) {
    uint8_t ret;
    atkbc_sendrec_t data;
    data.channel = 1;    /* second PS/2 port    */
    data.send    = 0;    /* don't send commands */
    data.sbuff   = NULL; /* ignored             */
    data.receive = 1;    /* I expect one byte   */
    data.rbuff   = &ret; /* receive buffer      */
    dev_ioctl(dev->parent_bus->ctl, i8042_ATKBC_SENDREC, &data);
    return ret;
}

static uint8_t enable_mouse(device_t *dev) {
    /* Send 0xF4 to the mouse. We should receive 0xFA (ACK). */
    uint8_t cmd = 0xF4, ret;
    atkbc_sendrec_t data;
    data.channel = 1;    /* second PS/2 port    */
    data.send    = 1;    /* send command.       */
    data.sbuff   = &cmd; /* command buffer      */
    data.receive = 1;    /* I expect one byte   */
    data.rbuff   = &ret; /* receive buffer      */
    dev_ioctl(dev->parent_bus->ctl, i8042_ATKBC_SENDREC, &data);
    return ret;
}

/* ================================================================= */
/*                             Interface                             */
/* ================================================================= */

uint32_t ps2mouse_probe(device_t *dev, void *config) {

    /* inform the user that the driver is loading */
    printk("PS/2 Mouse driver is initializing...\n");

    /* initialize cursor position */
    mouse_x = bootinfo->vga_width/2;
    mouse_y = bootinfo->vga_height/2;

    /* enable the mouse */
    enable_mouse(dev);

    /* catch the IRQs: */
    irq_reserve(dev->resources.list[0].data.irq.number, dev, 0);

    /* done */
    return ESUCCESS;

}

uint32_t ps2mouse_read(device_t *dev, uint64_t off, uint32_t size, char *buff){
    return ESUCCESS;
}

uint32_t ps2mouse_write(device_t *dev, uint64_t off, uint32_t size,char *buff){
    return ESUCCESS;
}

uint32_t ps2mouse_ioctl(device_t *dev, uint32_t cmd, void *data) {

    switch (cmd) {
        case MOUSE_GETX:
            *((int32_t *) data) = mouse_x;
            break;
        case MOUSE_GETY:
            *((int32_t *) data) = mouse_y;
            break;
        case MOUSE_REG:
            listener_pid    = curproc->pid;
            listener_prefix = (uint8_t) data;
            break;
        default:
            break;
    }

    return ESUCCESS;

}

uint32_t ps2mouse_irq(device_t *dev, uint32_t irqn) {

    unsigned char packet = read_packet(dev);
    int dx, dy;

    if (mouse_counter == 0) {
        /* Byte 1 */
        left_btn  = packet & 0x01;
        right_btn = packet & 0x02;
        mid_btn   = packet & 0x04;
        x_sign    = packet & 0x10;
        y_sign    = packet & 0x20;
        x_of      = packet & 0x40;
        y_of      = packet & 0x80;
        /*printk("X sign: %d, Y sign: %d\n", x_sign, y_sign);*/
    } else if (mouse_counter == 1) {
        /* Byte 2 */
        if (x_of)
            return ESUCCESS;
        if (!x_sign) {
            dx = packet;
        } else {
            dx = ((int)packet) - 0x100;
        }
        if (!legacy_lfb_enabled)
            mouse_x += dx;
        /*printk("x packet: %d, dx: %x\n", (unsigned char) packet, dx);*/
    } else {
        /* Byte 3 */
        if (y_of)
            return ESUCCESS;
        if (!y_sign) {
            dy = packet;
        } else {
            dy = ((int)packet) - 0x100;
        }
        if (!legacy_lfb_enabled)
            mouse_y -= dy;
        /*printk("y packet: %d, dy: %x\n", (unsigned char) packet, dy);
        printk("===============\n");*/
    }

    if (mouse_x < 0)
        mouse_x = 0;
    else if (mouse_x >= bootinfo->vga_width)
        mouse_x = bootinfo->vga_width-1;

    if (mouse_y < 0)
        mouse_y = 0;
    else if (mouse_y >= bootinfo->vga_height)
        mouse_y = bootinfo->vga_height-1;

    if (mouse_counter == 2 && listener_pid != -1 && !legacy_lfb_enabled) {

        /* send a packet to the listening program */
        mouse_packet_t packet;
        msg_t msg;
        extern uint64_t ticks;

        /* initialize packet */
        packet.prefix    = listener_prefix;
        packet.mouse_x   = mouse_x;
        packet.mouse_y   = mouse_y;
        packet.left_btn  = left_btn;
        packet.mid_btn   = mid_btn;
        packet.right_btn = right_btn;
        packet.time      = ticks*10;

        /* initialize msg */
        msg.size = sizeof(packet);
        msg.buf  = &packet;

        /* do the send! */
        send(listener_pid, &msg);

    }

    mouse_counter = (mouse_counter + 1)%3;

    return ESUCCESS;
}
