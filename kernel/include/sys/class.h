/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Device classification system.                    | |
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

#ifndef CLASS_H
#define CLASS_H

#include <arch/type.h>

/* ============================================================== */
/*                          Bus Codes                             */
/* ============================================================== */

#define BUS_NOBUS                       0x00
#define BUS_GENESIS                     0x01
#define BUS_MEMORY                      0x02
#define BUS_HOST                        0x03
#define BUS_PCI                         0x04
#define BUS_ISA                         0x05
#define BUS_SERIO                       0x06
#define BUS_IDE                         0x07
#define BUS_SCSI                        0x08
#define BUS_PCMCIA                      0x09
#define BUS_CARDBUS                     0x0A
#define BUS_USB                         0x0B
#define BUS_DISK                        0x0C
#define BUS_UNDEFINED                   0xFF

/* ============================================================== */
/*                          NoBus Codes                           */
/* ============================================================== */

#define BASE_NOBUS_ROOT                 0x00

/* ============================================================== */
/*                       Genesis Bus Codes                        */
/* ============================================================== */

#define BASE_GENESIS_MEMORY             0x00
#define  SUB_GENESIS_MEMORY             0x00

#define BASE_GENESIS_TTY                0x01
#define  SUB_GENESIS_TTY_VIRTUAL        0x00
#define  SUB_GENESIS_TTY_PSEUDO         0x01

#define BASE_GENESIS_MACHINE            0x02
#define  SUB_GENESIS_MACHINE_IBMPC      0x00

/* ============================================================== */
/*                    Virtual Memory Bus Codes                    */
/* ============================================================== */

#define BASE_MEMORY_NODE                0x00
#define  SUB_MEMORY_NODE_NULL           0x00
#define  SUB_MEMORY_NODE_ZERO           0x01
#define  SUB_MEMORY_NODE_FULL           0x02
#define  SUB_MEMORY_NODE_MEM            0x03
#define  SUB_MEMORY_NODE_PORT           0x04
#define  SUB_MEMORY_NODE_RANDOM         0x05
#define  SUB_MEMORY_NODE_URANDOM        0x06
#define  SUB_MEMORY_NODE_RAMDISK        0x07

/* ============================================================== */
/*                        Host Bus Codes                          */
/* ============================================================== */

#define BASE_HOST_CPU                   0x00
#define  SUB_HOST_CPU_386               0x00
#define  SUB_HOST_CPU_ARM               0x01

#define BASE_HOST_SUBSYSTEM             0x01
#define  SUB_HOST_SUBSYSTEM_PCI         0x00
#define  SUB_HOST_SUBSYSTEM_ISA         0x01

/* ============================================================== */
/*                       PCI Class Codes                          */
/* ============================================================== */

#define BASE_PCI_PRE2                   0x00
#define  SUB_PCI_PRE2_NOTVGA            0x00
#define  SUB_PCI_PRE2_VGA               0x01

#define BASE_PCI_STORAGE                0x01
#define  SUB_PCI_STORAGE_SCSI           0x00
#define  SUB_PCI_STORAGE_IDE            0x01
#define  SUB_PCI_STORAGE_FLOPPY         0x02
#define  SUB_PCI_STORAGE_IPI            0x03
#define  SUB_PCI_STORAGE_RAID           0x04
#define  SUB_PCI_STORAGE_ATA            0x05
#define   IF_PCI_STORAGE_ATA_SINGLE     0x20
#define   IF_PCI_STORAGE_ATA_CHAINED    0x30
#define  SUB_PCI_STORAGE_SATA           0x06
#define   IF_PCI_STORAGE_SATA_SPECIFIC  0x00
#define   IF_PCI_STORAGE_SATA_AHCI      0x01
#define  SUB_PCI_STORAGE_OTHER          0x80

#define BASE_PCI_NETWORK                0x02
#define  SUB_PCI_NETWORK_ETHERNET       0x00
#define  SUB_PCI_NETWORK_TOKEN_RING     0x01
#define  SUB_PCI_NETWORK_FDDI           0x02
#define  SUB_PCI_NETWORK_ATM            0x03
#define  SUB_PCI_NETWORK_OTHER          0x80

#define BASE_PCI_DISPLAY                0x03
#define  SUB_PCI_DISPLAY_VGA            0x00
#define  SUB_PCI_DISPLAY_XGA            0x01
#define  SUB_PCI_DISPLAY_OTHER          0x80

#define BASE_PCI_MULTIMEDIA             0x04
#define  SUB_PCI_MULTIMEDIA_VIDEO       0x00
#define  SUB_PCI_MULTIMEDIA_AUDIO       0x01
#define  SUB_PCI_MULTIMEDIA_OTHER       0x80

#define BASE_PCI_MEMORY                 0x05
#define  SUB_PCI_MEMORY_RAM             0x00
#define  SUB_PCI_MEMORY_FLASH           0x01
#define  SUB_PCI_MEMORY_OTHER           0x80

#define BASE_PCI_BRIDGE                 0x06
#define  SUB_PCI_BRIDGE_HOST_TO_PCI     0x00
#define  SUB_PCI_BRIDGE_PCI_TO_ISA      0x01
#define  SUB_PCI_BRIDGE_PCI_TO_EISA     0x02
#define  SUB_PCI_BRIDGE_PCI_TO_MICRO    0x03
#define  SUB_PCI_BRIDGE_PCI_TO_PCI      0x04
#define  SUB_PCI_BRIDGE_PCI_TO_PCMCIA   0x05
#define  SUB_PCI_BRIDGE_PCI_TO_NUBUS    0x06
#define  SUB_PCI_BRIDGE_PCI_TO_CARDBUS  0x07
#define  SUB_PCI_BRIDGE_OTHER           0x80

#define BASE_PCI_COMMUNICATION          0x07
#define  SUB_PCI_COMMUNICATION_SERIAL   0x00
#define  SUB_PCI_COMMUNICATION_PARALLEL 0x01
#define  SUB_PCI_COMMUNICATION_OTHER    0x80

#define BASE_PCI_PERIPHERALS            0x08
#define  SUB_PCI_PERIPHERALS_PIC        0x00
#define  SUB_PCI_PERIPHERALS_DMA        0x01
#define  SUB_PCI_PERIPHERALS_TIMER      0x02
#define  SUB_PCI_PERIPHERALS_RTC        0x03
#define  SUB_PCI_PERIPHERALS_OTHER      0x80

#define BASE_PCI_INPUT                  0x09
#define  SUB_PCI_INPUT_KEYBOARD         0x00
#define  SUB_PCI_INPUT_DIGITIZER        0x01
#define  SUB_PCI_INPUT_MOUSE            0x02
#define  SUB_PCI_INPUT_OTHER            0x80

#define BASE_PCI_DOCKING                0x0A
#define  SUB_PCI_DOCKING_GENERIC        0x00
#define  SUB_PCI_DOCKING_OTHER          0x80

#define BASE_PCI_PROCESSOR              0x0B
#define  SUB_PCI_PROCESSOR_386          0x00
#define  SUB_PCI_PROCESSOR_486          0x01
#define  SUB_PCI_PROCESSOR_PENTIUM      0x02
#define  SUB_PCI_PROCESSOR_ALPHA        0x10
#define  SUB_PCI_PROCESSOR_POWERPC      0x20
#define  SUB_PCI_PROCESSOR_COPROCESSOR  0x40

#define BASE_PCI_SERIALBUS              0x0C
#define  SUB_PCI_SERIALBUS_FIREWIRE     0x00
#define  SUB_PCI_SERIALBUS_ACCESS       0x01
#define  SUB_PCI_SERIALBUS_SSA          0x02
#define  SUB_PCI_SERIALBUS_USB          0x03
#define   IF_PCI_SERIALBUS_USB_UHCI     0x00
#define   IF_PCI_SERIALBUS_USB_OHCI     0x10
#define   IF_PCI_SERIALBUS_USB_EHCI     0x20
#define   IF_PCI_SERIALBUS_USB_XHCI     0x30

/* ============================================================== */
/*                       ISA Class Codes                          */
/* ============================================================== */

#define BASE_ISA_INTEL                  0x8086
#define  SUB_ISA_INTEL_8259A            0x0000
#define  SUB_ISA_INTEL_8253             0x0001
#define  SUB_ISA_INTEL_8042             0x0002

#define BASE_ISA_IBM                    0x8087
#define  SUB_ISA_IBM_VGA                0x0000

/* ============================================================== */
/*                       Serio Bus Codes                          */
/* ============================================================== */

#define BASE_i8042_ATKBC                0x0000
#define  SUB_i8042_ATKBC_PS2KEYBOARD    0x0000
#define  SUB_i8042_ATKBC_PS2MOUSE       0x0001

/* ============================================================== */
/*                        IDE Bus Codes                           */
/* ============================================================== */

#define BASE_ATA_DISK                   0x0000
#define  SUB_ATA_DISK_GENERIC           0x0000

#define BASE_ATAPI_CDROM                0x0001
#define  SUB_ATAPI_CDROM_GENERIC        0x0000

/* ============================================================== */
/*                        SCSI Bus Codes                          */
/* ============================================================== */

#define BASE_SCSI_DISK                  0x0000
#define  SUB_SCSI_DISK_GENERIC          0x0000

/* ============================================================== */
/*                        USB Bus Codes                           */
/* ============================================================== */

#define BASE_USB_MASS_STORAGE           0x08
#define  SUB_USB_MASS_STORAGE_SCSI      0x06

#define BASE_USB_HUB                    0x09
#define  SUB_USB_HUB                    0x00
#define   IF_USB_HUB_FULLSPEED          0x00
#define   IF_USB_HUB_HISPEED_SINGLE_TT  0x01
#define   IF_USB_HUB_HISPEED_MULTI_TT   0x02
#define   IF_USB_HUB_ROOTHUB            0xFF

/* ============================================================== */
/*                        DISK Bus Codes                          */
/* ============================================================== */

#define BASE_DISK_PARTITION             0x0000
#define  SUB_DISK_PARTITION_ID0         0x0000
#define   IF_DISK_PARTITION_ID0_DEVID0  0x0000

/* ============================================================== */
/*                          Don't Care                            */
/* ============================================================== */

#define BASE_ANY                        0xFFFFFFFF
#define  SUB_ANY                        0xFFFFFFFF
#define   IF_ANY                        0xFFFFFFFF

/* ============================================================== */
/*                          Structures                            */
/* ============================================================== */

typedef struct {
    uint32_t bus;       /* bus type.              */
    uint32_t base;      /* base class.            */
    uint32_t sub;       /* sub class.             */
    uint32_t progif;    /* programming interface. */
} class_t;

#endif
