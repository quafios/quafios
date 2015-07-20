/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Window Manager.                             | |
 *        | |  -> Inbox procedures.                                | |
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

#include <mouse/generic.h>
#include <keyboard/generic.h>
#include <api/proc.h>

#include "winman.h"

void inbox_listen() {

    /* message structures */
    msg_t msg;
    unsigned char packet[1024];

    /* initialize msg */
    msg.buf = &packet;

    /* check the inbox */
    while (1) {

        receive(&msg, 1);

        if (packet[0] == PREFIX_MOUSE)
            mouse_event((mouse_packet_t *) &packet);
        else if (packet[0] == PREFIX_KEYBOARD)
            keyboard_event((keyboard_packet_t *) &packet);
        else if (packet[0] == PREFIX_APPLICATION)
            window_request(msg.sender, packet);

    }

}

void inbox_init() {

    /* initialize inbox */
    FILE *f = fopen("/run/winman", "w");
    fprintf(f, "%d\n", getpid());
    fclose(f);

}
