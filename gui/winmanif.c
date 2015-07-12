/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios GUI Library.                                | |
 *        | |  -> Window manager interface.                        | |
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
#include <api/proc.h>
#include <api/fs.h>

void (*receiver)(void *packet) = NULL;

int winman_pid = 0;

int wm_req(void *req, int size) {
    msg_t msg;
    if (winman_pid == 0) {
        int fd = open("/run/winman", 0);
        char c;
        if (fd < 1)
            return -1;
        while (read(fd, &c, 1) && c >= '0' && c <= '9') {
            winman_pid = winman_pid*10 + c-'0';
        }
        close(fd);
    }
    msg.buf = req;
    msg.size = size;
    return send(winman_pid, &msg);
}

void set_receiver(void (*rec)(void *packet)) {
    receiver = rec;
}

void gui_loop() {

    /* message structures */
    msg_t msg;
    unsigned char packet[256];
    extern window_t *firstwin;

    /* initialize msg */
    msg.buf = &packet;

    /* check the inbox */
    while (1) {

        if (!firstwin) {
            /* all windows are closed */
            return;
        }

        receive(&msg, 1);

        if (packet[0] == PREFIX_WINMAN) {
            /* an event from winman */
            wm_event_t *event = (wm_event_t *) &packet;
            window_t *win = get_window(event->id);

            if (win) {
                switch(event->type) {
                    case WMEVENT_PRESS:
                        window_press(win, event->data[0]);
                        break;
                    case WMEVENT_CLICK:
                        window_click(win, event->data[0], event->data[1]);
                        break;
                    case WMEVENT_DOUBLE:
                        window_double(win, event->data[0], event->data[1]);
                        break;
                    case WMEVENT_CLOSE:
                        window_close(win);
                        break;
                    default:
                        break;
                }
            }

        } else if (receiver) {
            /* unknown event */
            receiver(packet);
        }

    }

}
