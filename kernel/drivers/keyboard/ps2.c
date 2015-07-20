/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> PS/2 Keyboard Device Driver.                     | |
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

/* currently, only old XT keyboard scancode set is supported. */

#include <arch/type.h>
#include <arch/irq.h>
#include <lib/linkedlist.h>
#include <sys/error.h>
#include <sys/printk.h>
#include <sys/mm.h>
#include <sys/class.h>
#include <sys/resource.h>
#include <sys/device.h>
#include <serio/8042kbc.h>
#include <tty/vtty.h>
#include <keyboard/generic.h>
#include <sys/scheduler.h>

/* Prototypes: */
uint32_t ps2kbd_probe(device_t *, void *);
uint32_t ps2kbd_read (device_t *, uint64_t, uint32_t, char *);
uint32_t ps2kbd_write(device_t *, uint64_t, uint32_t, char *);
uint32_t ps2kbd_ioctl(device_t *, uint32_t, void *);
uint32_t ps2kbd_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_SERIO, BASE_i8042_ATKBC, SUB_i8042_ATKBC_PS2KEYBOARD, IF_ANY}
};

/* driver_t structure that identifies this driver: */
driver_t ps2_keyboard = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "ps2_keyboard",
    /* probe:     */ ps2kbd_probe,
    /* read:      */ ps2kbd_read,
    /* write:     */ ps2kbd_write,
    /* ioctl:     */ ps2kbd_ioctl,
    /* irq:       */ ps2kbd_irq
};

typedef struct {
    uint32_t num_lock;
    uint32_t caps_lock;
    uint32_t scroll_lock;

    uint32_t Control;
    uint32_t Alt;
    uint32_t Left_Shift;
    uint32_t Right_Shift;

    uint32_t escaped;

    device_t *dev;
} info_t;

static int32_t listener_pid = -1;
static uint8_t listener_prefix = -1;
extern uint32_t legacy_lfb_enabled;

#define NON     0x00 /* none         */
#define KEY     0x01 /* normal key   */
#define NPD     0x02 /* number pad   */
#define CTR     0x03 /* control key  */
#define ALT     0x04 /* alt key      */
#define LSH     0x05 /* left shift   */
#define RSH     0x06 /* right shift  */
#define NUM     0x07 /* num lock     */
#define CPS     0x08 /* caps lock    */
#define SCR     0x09 /* scroll lock  */
#define FUN     0x0A /* function key */
#define ESC     0x0B /* escaped key  */

static const char map[][3] = {
    /*00*/{  0 ,  0 , NON},/*01*/{ 27 , 27 , KEY},/*02*/{ '1', '!', KEY},
    /*03*/{ '2', '@', KEY},/*04*/{ '3', '#', KEY},/*05*/{ '4', '$', KEY},
    /*06*/{ '5', '%', KEY},/*07*/{ '6', '^', KEY},/*08*/{ '7', '&', KEY},
    /*09*/{ '8', '*', KEY},/*0A*/{ '9', '(', KEY},/*0B*/{ '0', ')', KEY},
    /*0C*/{ '-', '_', KEY},/*0D*/{ '=', '+', KEY},/*0E*/{  8 ,  8 , KEY},
    /*0F*/{  9 ,  9 , KEY},/*10*/{ 'q', 'Q', KEY},/*11*/{ 'w', 'W', KEY},
    /*12*/{ 'e', 'E', KEY},/*13*/{ 'r', 'R', KEY},/*14*/{ 't', 'T', KEY},
    /*15*/{ 'y', 'Y', KEY},/*16*/{ 'u', 'U', KEY},/*17*/{ 'i', 'I', KEY},
    /*18*/{ 'o', 'O', KEY},/*19*/{ 'p', 'P', KEY},/*1A*/{ '[', '{', KEY},
    /*1B*/{ ']', '}', KEY},/*1C*/{'\n','\n', KEY},/*1D*/{  0 ,  0 , CTR},
    /*1E*/{ 'a', 'A', KEY},/*1F*/{ 's', 'S', KEY},/*20*/{ 'd', 'D', KEY},
    /*21*/{ 'f', 'F', KEY},/*22*/{ 'g', 'G', KEY},/*23*/{ 'h', 'H', KEY},
    /*24*/{ 'j', 'J', KEY},/*25*/{ 'k', 'K', KEY},/*26*/{ 'l', 'L', KEY},
    /*27*/{ ';', ':', KEY},/*28*/{ 39 , '"', KEY},/*29*/{ '`', '~', KEY},
    /*2A*/{  0 ,  0 , LSH},/*2B*/{ 92 , '|', KEY},/*2C*/{ 'z', 'Z', KEY},
    /*2D*/{ 'x', 'X', KEY},/*2E*/{ 'c', 'C', KEY},/*2F*/{ 'v', 'V', KEY},
    /*30*/{ 'b', 'B', KEY},/*31*/{ 'n', 'N', KEY},/*32*/{ 'm', 'M', KEY},
    /*33*/{ ',', '<', KEY},/*34*/{ '.', '>', KEY},/*35*/{ '/', '?', KEY},
    /*36*/{  0 ,  0 , RSH},/*37*/{ '*', '*', KEY},/*38*/{  0 ,  0 , ALT},
    /*39*/{ ' ', ' ', KEY},/*3A*/{  0 ,  0 , CPS},/*3B*/{  0 ,  0 , FUN},
    /*3C*/{  0 ,  0 , FUN},/*3D*/{  0 ,  0 , FUN},/*3E*/{  0 ,  0 , FUN},
    /*3F*/{  0 ,  0 , FUN},/*40*/{  0 ,  0 , FUN},/*41*/{  0 ,  0 , FUN},
    /*42*/{  0 ,  0 , FUN},/*43*/{  0 ,  0 , FUN},/*44*/{  0 ,  0 , FUN},
    /*45*/{  0 ,  0 , NUM},/*46*/{  0 ,  0 , SCR},/*47*/{ 18 , '7', NPD},
    /*48*/{ 23 , '8', NPD},/*49*/{ 20 , '9', NPD},/*4A*/{ '-', '-', KEY},
    /*4B*/{ 26 , '4', NPD},/*4C*/{ 22 , '5', NPD},/*4D*/{ 25 , '6', NPD},
    /*4E*/{ '+', '+', KEY},/*4F*/{ 19 , '1', NPD},/*50*/{ 24 , '2', NPD},
    /*51*/{ 21 , '3', NPD},/*52*/{ 16 , '0', NPD},/*53*/{ 17 , '.', NPD},
    /*54*/{  0 ,  0 , NON},/*55*/{  0 ,  0 , NON},/*56*/{ '<', '>', KEY},
    /*57*/{  0 ,  0 , FUN},/*58*/{  0 ,  0 , FUN},/*59*/{  0 ,  0 , FUN},
    /*5A*/{  0 ,  0 , FUN},/*5B*/{0x0E,0x0E, NON},/*5C*/{0x0E,0x0E, NON},
    /*5D*/{0x0F,0x0F, NON},/*5E*/{  0 ,  0 , NON},/*5F*/{  0 ,  0 , NON},
    /*60*/{  0 ,  0 , NON},/*61*/{  0 ,  0 , NON},/*62*/{  0 ,  0 , NON},
    /*63*/{  0 ,  0 , NON},/*64*/{  0 ,  0 , NON},/*65*/{  0 ,  0 , NON},
    /*66*/{  0 ,  0 , NON},/*67*/{  0 ,  0 , NON},/*68*/{  0 ,  0 , NON},
    /*69*/{  0 ,  0 , NON},/*6A*/{  0 ,  0 , NON},/*6B*/{  0 ,  0 , NON},
    /*6C*/{  0 ,  0 , NON},/*6D*/{  0 ,  0 , NON},/*6E*/{  0 ,  0 , NON},
    /*6F*/{  0 ,  0 , NON},/*70*/{  0 ,  0 , NON},/*71*/{  0 ,  0 , NON},
    /*72*/{  0 ,  0 , NON},/*73*/{  0 ,  0 , NON},/*74*/{  0 ,  0 , NON},
    /*75*/{  0 ,  0 , NON},/*76*/{  0 ,  0 , NON},/*77*/{  0 ,  0 , NON},
    /*78*/{  0 ,  0 , NON},/*79*/{  0 ,  0 , NON},/*7A*/{  0 ,  0 , NON},
    /*7B*/{  0 ,  0 , NON},/*7C*/{  0 ,  0 , NON},/*7D*/{  0 ,  0 , NON},
    /*7E*/{  0 ,  0 , NON},/*7F*/{  0 ,  0 , NON}
};

/* ================================================================= */
/*                             Commands                              */
/* ================================================================= */

static uint8_t read_scancode(info_t *info) {
    /* only used when a scan code is expected to be already sent */
    uint8_t ret;
    atkbc_sendrec_t data;
    data.channel = 0;    /* first PS/2 port     */
    data.send    = 0;    /* don't send commands */
    data.sbuff   = NULL; /* ignored             */
    data.receive = 1;    /* I expect one byte   */
    data.rbuff   = &ret; /* receive buffer      */
    dev_ioctl(info->dev->parent_bus->ctl, i8042_ATKBC_SENDREC, &data);
    return ret;
}

static void update_leds(info_t *info) {
    uint8_t buff[2];
    atkbc_sendrec_t data;
    buff[0] = 0xED; /* SET LED Status */
    buff[1] = (info->caps_lock   * 4) +
              (info->num_lock    * 2) +
              (info->scroll_lock * 1);
    data.channel = 0;    /* first PS/2 port */
    data.send    = 2;    /* send 2 bytes    */
    data.sbuff   = buff; /* the buffer      */
    data.receive = 0;    /* Receive nothing */
    data.rbuff   = NULL;
    dev_ioctl(info->dev->parent_bus->ctl, i8042_ATKBC_SENDREC, &data);
}

/* ================================================================= */
/*                           Key handling                            */
/* ================================================================= */

static void invoke_event(info_t *info, uint8_t chr) {

    /* Is the system in text mode or vga mode? */
    if (legacy_lfb_enabled) {
        /* print the character to console: */
        if (system_console != (void *) 0)
            dev_ioctl((device_t *) system_console, TTY_PRESS, &chr);
    } else {
        /* send the event to the listening process */
        if (listener_pid != -1) {
            keyboard_packet_t packet;
            msg_t msg;

            /* initialize packet */
            packet.prefix    = listener_prefix;
            packet.button    = chr;
            packet.alt       = info->Alt;
            packet.control   = info->Control;

            /* initialize msg */
            msg.size = sizeof(packet);
            msg.buf  = &packet;

            /* do the send! */
            send(listener_pid, &msg);
        }
    }

}

static void key(info_t *info, uint8_t scancode){
    /* a character key is pressed.. */
    char chr;
    uint32_t caps_lock = info->caps_lock;
    uint32_t Left_Shift = info->Left_Shift;
    uint32_t Right_Shift = info->Right_Shift;

    if (scancode & 0x80) return;

    /* See if the latin keys are capitalized or not:
     * Shift = 0 && Caps = 0 ; Small
     * Shift = 1 && Caps = 0 ; Capital
     * Shift = 0 && Caps = 1 ; Capital
     * Shift = 1 && Caps = 1 ; Small
     */

    if (((scancode > 0x0F) && (scancode < 0x1A)) ||
        ((scancode > 0x1D) && (scancode < 0x27)) ||
        ((scancode > 0x2B) && (scancode < 0x33)))
            chr = map[scancode][(caps_lock != (Left_Shift||Right_Shift))];
    else
            chr = map[scancode][(Left_Shift||Right_Shift)];

    /* invoke an event */
    invoke_event(info, chr);

}

static void npd(info_t *info, uint8_t scancode){
    /* Number Pad Key... */

    /* See if the keys of the num pad are Numbers or
     * Arrows & Gray Controls
     * Num_Lock = 0 && Shift = 0; arrows
     * Num_Lock = 1 && Shift = 0; numbers
     * Num_Lock = 0 && Shift = 1; numbers
     * Num_Lock = 1 && Shift = 1;arrows
     */

    char chr;
    uint32_t num_lock = info->num_lock;
    uint32_t Left_Shift = info->Left_Shift;
    uint32_t Right_Shift = info->Right_Shift;

    if (scancode & 0x80) return;

    chr = map[scancode][num_lock != (Left_Shift || Right_Shift)];

    /* print the character to console: */
    invoke_event(info, chr);

}

static void ctr(info_t *info, uint8_t scancode){
    if (scancode & 0x80)
        info->Control = 0;
    else
        info->Control = 1;
}

static void alt(info_t *info, uint8_t scancode){
    if (scancode & 0x80)
        info->Alt = 1;
    else
        info->Alt = 0;
}

static void lsh(info_t *info, uint8_t scancode){
    if (scancode & 0x80)
        info->Left_Shift = 0;
    else
        info->Left_Shift = 1;
}

static void rsh(info_t *info, uint8_t scancode){
    if (scancode & 0x80)
        info->Right_Shift = 0;
    else
        info->Right_Shift = 1;
}

static void num(info_t *info, uint8_t scancode) {
    if ((scancode & 0x80) == 0) {
        info->num_lock = !(info->num_lock);
        update_leds(info);
    }
}

static void cps(info_t *info, uint8_t scancode) {
    if ((scancode & 0x80) == 0) {
        info->caps_lock = !(info->caps_lock);
        update_leds(info);
    }
}

static void scr(info_t *info, uint8_t scancode) {
    if ((scancode & 0x80) == 0) {
        info->scroll_lock = !(info->scroll_lock);
        update_leds(info);
    }
}

static void fun(info_t *info, uint8_t scancode){
    /* From F1 to F15. */

    /* F1 or F2 pressed */
    if (scancode == 0x3B) {
        /* switch to text mode */
        vtty_ioctl(system_console, TTY_ENABLE, NULL);
    } else if (scancode == 0x3C) {
        /* switch to graphics mode */
        vtty_ioctl(system_console, TTY_DISABLE, NULL);
        invoke_event(info, 0); /* tell the window manager */
    }

}

static void esc(info_t *info, uint8_t scancode){
    /* Escape Characters (Important) */
    uint8_t code = scancode & 0x7F;
    info->escaped = 0;

    if (code == 0x1D)
        ctr(info, scancode); /* Right Control    */
    if (code == 0x2A)
        lsh(info, scancode); /* Fake Left  Shift */
    if (code == 0x36)
        rsh(info, scancode); /* Fake Right Shift */
    if (code == 0x38)
        alt(info, scancode); /* Right Alt        */

    if (code > 0x46 && code < 0x54)
        /* FIXME: 3ala el real machine 7amada
         *        w fel emulators 7amada tany 5ales.
         */
        npd(info, scancode); /* Grey Controls, Arrows */

    if (code == 0x1C || code == 0x35 || (code > 0x5A && code < 0x5E))
        key(info, scancode); /* Enter, /, Windows, Menu. */

    if ((scancode & 0x80) == 1) return;

    /* if (scancode == 0x37); */ /* Print Screen    */
    /* if (scancode == 0x46); */ /* Control - Break */
    if (code == 0x5E) {
        /* TODO: kbd_reboot(); */
    }
    /* if (scancode == 0x5F); */ /* Sleep */
    /* if (scancode == 0x63); */ /* Wake  */
}

static void handle(info_t *info, uint8_t scancode) {
    /* handle scancode... */

    /* escaped keys have a special treatment: */
    if (scancode == 0xE0 || scancode == 0xE1) {
        info->escaped = 1;
        return;
    }
    if (info->escaped == 1) {
        esc(info, scancode);
        return;
    }

    /* rest types: */
    switch(map[scancode & 0x7F][2]) {
        case KEY:
            key(info, scancode);
            break;
        case NPD:
            npd(info, scancode);
            break;
        case CTR:
            ctr(info, scancode);
            break;
        case ALT:
            alt(info, scancode);
            break;
        case LSH:
            lsh(info, scancode);
            break;
        case RSH:
            rsh(info, scancode);
            break;
        case NUM:
            num(info, scancode);
            break;
        case CPS:
            cps(info, scancode);
            break;
        case SCR:
            scr(info, scancode);
            break;
        case FUN:
            fun(info, scancode);
            break;
        default:
            break;
    }
}

/* ================================================================= */
/*                             Interface                             */
/* ================================================================= */

uint32_t ps2kbd_probe(device_t *dev, void *config) {

    irq_reserve_t *reserve;

    /* Allocate info_t: */
    info_t *info = (info_t *) kmalloc(sizeof(info_t));
    dev->drvreg = (uint32_t) info;
    if (info == NULL)
        return ENOMEM; /* i am sorry :D */

    /* initialize it: */
    info->num_lock = 0;
    info->caps_lock = 0;
    info->scroll_lock = 0;
    info->Control = 0;
    info->Alt = 0;
    info->Left_Shift = 0;
    info->Right_Shift = 0;
    info->escaped = 0;
    info->dev = dev;

    printk("PS/2 Keyboard driver is initializing...\n");

    /* no initialization is to be done.. the i8042 driver and
     * the ATKBC firmware driver did everything needed.
     */

    /* catch the IRQs: */
    reserve = kmalloc(sizeof(irq_reserve_t));
    reserve->dev     = dev;
    reserve->expires = 0;
    reserve->data    = NULL;
    irq_reserve(dev->resources.list[0].data.irq.number, reserve);

    /* make sure le buffer is clear (v. important): */
    read_scancode(info);

    /* update leds: */
    update_leds(info);

    return ESUCCESS;
}

uint32_t ps2kbd_read(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    return ESUCCESS;
}

uint32_t ps2kbd_write(device_t *dev, uint64_t off, uint32_t size, char *buff) {
    return ESUCCESS;
}

uint32_t ps2kbd_ioctl(device_t *dev, uint32_t cmd, void *data) {

    switch (cmd) {

        case KEYBOARD_REG:
            listener_pid    = curproc->pid;
            listener_prefix = (uint8_t) data;
            break;

        default:
            break;

    }

    return ESUCCESS;
}

uint32_t ps2kbd_irq(device_t *dev, uint32_t irqn) {

    /* get info_t: */
    info_t *info = (info_t *) dev->drvreg;
    if (info == NULL)
            return ENOMEM;

    /* Read & handle the scan code immediately: */
    handle(info, read_scancode(info));

    return ESUCCESS;
}
