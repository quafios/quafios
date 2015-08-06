/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> USB hub header.                                  | |
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

#ifndef USB_HUB_H
#define USB_HUB_H

/* USB hub additional standard requests */
#define GET_STATE               2

/* hub features */
#define C_HUB_LOCAL_POWER       0
#define C_HUB_OVER_CURRENT      1

/* port features */
#define PORT_CONNECTION         0
#define PORT_ENABLE             1
#define PORT_SUSPEND            2
#define PORT_OVER_CURRENT       3
#define PORT_RESET              4
#define PORT_POWER              8
#define PORT_LOW_SPEED          9
#define C_PORT_CONNECTION       16
#define C_PORT_ENABLE           17
#define C_PORT_SUSPEND          18
#define C_PORT_OVER_CURRENT     19
#define C_PORT_RESET            20

/* hub descriptor */
typedef struct hub_descriptor {
    uint8_t  bDescLength;
#define HUB_DESCRIPTOR          0x29
    uint8_t  bDescriptorType;
    uint8_t  bNbrPorts;
    uint16_t wHubCharacteristics;
    uint8_t  bPwrOn2PwrGood;
    uint8_t  bHubContrCurrent;
    uint8_t  DeviceRemovable_PortPwrCtrlMask[1];
} hub_descriptor_t;

/* port status field */
typedef struct wPortStatus {
    unsigned port_connection     : 1;
    unsigned port_enable         : 1;
    unsigned port_suspend        : 1;
    unsigned port_over_current   : 1;
    unsigned port_reset          : 1;
    unsigned reserved1           : 3;
    unsigned port_power          : 1;
    unsigned port_low_speed      : 1;
    unsigned port_high_speed     : 1;
    unsigned port_test_mode      : 1;
    unsigned port_indicator_ctrl : 1;
    unsigned reserved2           : 3;
} __attribute__((packed)) wPortStatus_t;

/* port change field */
typedef struct wPortChange {
    unsigned c_port_connection   : 1;
    unsigned c_port_enable       : 1;
    unsigned c_port_suspend      : 1;
    unsigned c_port_over_current : 1;
    unsigned c_port_reset        : 1;
    unsigned reserved            : 11;
} __attribute__((packed)) wPortChange_t;

#endif
