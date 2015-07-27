/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Boot-Loader.                                | |
 *        | |  -> A20 line enabling code.                          | |
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
#include <i386/asm.h>

#define IOPORT_DATA   0x60
#define IOPORT_CMD    0x64

#define STATUS_OBF    1    /* Output buffer full? */
#define STATUS_IBF    2    /* Input  buffer full? */

static uint8_t status() {
    return inb(IOPORT_CMD);
}

static void sendCMD(uint8_t cmd) {
    while(status() & STATUS_IBF); /* wait for the input buffer. */
    outb(cmd, IOPORT_CMD);
}

static uint16_t getData() {
    /* dunno why next line doesn't work well on virtualbox when
     * interrupts are enabled..
     */
    while(!(status() & STATUS_OBF)); /* wait for output buffer. */
    return inb(IOPORT_DATA);
}

static void sendData(uint8_t data) {
    while(status() & STATUS_IBF); /* wait for the input buffer. */
    outb(data, IOPORT_DATA);
}

void enable_a20() {

    /* data from controller output port */
    uint8_t ctl;

    /* disable interrupts */
    cli(); /* because virtualbox cries... */

    /* Disable PS/2 Port 1: */
    sendCMD(0xAD);

    /* Read Controller Output Port */
    sendCMD(0xD0);
    ctl = getData();

    /* Update Controller Output Port */
    sendCMD(0xD1);
    sendData(ctl | 0x02);

    /* Re-enable PS/2 Port 1: */
    sendCMD(0xAE);

    /* enable interrupts again */
    sti();

}

void disable_a20() {

    /* data from controller output port */
    uint8_t ctl;

    /* disable interrupts */
    cli();

    /* Disable PS/2 Port 1: */
    sendCMD(0xAD);

    /* Read Controller Output Port */
    sendCMD(0xD0);
    ctl = getData();

    /* Update Controller Output Port */
    sendCMD(0xD1);
    sendData(ctl & 0xFD);

    /* Re-enable PS/2 Port 1: */
    sendCMD(0xAE);

    /* enable interrupts: */
    sti();

}
