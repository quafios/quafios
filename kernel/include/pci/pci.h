/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> PCI bus header.                                  | |
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

#ifndef PCI_DRIVER_H
#define PCI_DRIVER_H

#include <arch/type.h>
#include <sys/device.h>

/* configuration space registers */
#define PCI_REG_VENDOR_ID       0x00
#define PCI_REG_DEVICE_ID       0x02
#define PCI_REG_COMMAND         0x04
#define PCI_REG_STATUS          0x06
#define PCI_REG_REVISION_ID     0x08
#define PCI_REG_PROG_IF         0x09
#define PCI_REG_SUBCLASS        0x0A
#define PCI_REG_CLASS           0x0B
#define PCI_REG_CACHE_LINE_SIZE 0x0C
#define PCI_REG_LATENCY_TIMER   0x0D
#define PCI_REG_HEADER_TYPE     0x0E
#define PCI_REG_BIST            0x0F
#define PCI_REG_BASE_ADDRESS_0  0x10
#define PCI_REG_BASE_ADDRESS_1  0x14
#define PCI_REG_BASE_ADDRESS_2  0x18
#define PCI_REG_BASE_ADDRESS_3  0x1C
#define PCI_REG_BASE_ADDRESS_4  0x20
#define PCI_REG_BASE_ADDRESS_5  0x24
#define PCI_REG_CARDBUS_CIS_PTR 0x28
#define PCI_REG_SUBSYS_VENDOR   0x2C
#define PCI_REG_SUBSYS_ID       0x2E
#define PCI_REG_EXPANSION_ROM   0x30
#define PCI_REG_CAP_PTR         0x34
#define PCI_REG_RESERVED_1      0x35
#define PCI_REG_RESERVED_4      0x38
#define PCI_REG_INTERRUPT_LINE  0x3C
#define PCI_REG_INTERRUPT_PIN   0x3D
#define PCI_REG_MIN_GRANT       0x3E
#define PCI_REG_MAX_LATENCY     0x3F

/* config structure */
typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t bus;
    uint8_t devno;
    uint8_t func;
    device_t *master;
} pci_config_t;

#endif
