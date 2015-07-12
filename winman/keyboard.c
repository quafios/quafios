/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Window Manager.                             | |
 *        | |  -> Keyboard interface.                              | |
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

#include <gui.h>
#include <api/fs.h>
#include <api/proc.h>
#include <keyboard/generic.h>

#include "winman.h"

int keyboard_fd;

void keyboard_event(keyboard_packet_t *packet) {

    win_info_t *top = get_top_window();

    if (packet->button == 0) {

        /* F2 pressed, redraw the screen */
        vga_plot(get_osm(), 0, 0);

        /* draw the mouse cursor */
        draw_cursor();

    } else if (get_top_window()) {

        /* send event to the top window */
        msg_t msg;
        wm_event_t event = {0};
        event.prefix  = PREFIX_WINMAN; /* winman packet */
        event.type    = WMEVENT_PRESS; /* event type                  */
        event.id      = top->wid;      /* local handler of the window */
        event.data[0] = packet->button;
        msg.buf = &event;
        msg.size = sizeof(wm_event_t);
        send(top->pid, &msg);

    }

}

void keyboard_init() {

    /* open the keyboard driver */
    keyboard_fd = open("/dev/keyboard", 0);

    /* register winman at the keyboard driver */
    ioctl(keyboard_fd, KEYBOARD_REG, (void *) PREFIX_KEYBOARD);

}
