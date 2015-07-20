/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> ATA bus header.                                  | |
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

#ifndef ATA_IDE_H
#define ATA_IDE_H

#include <sys/device.h>

/* ATA commands */
#define ATA_CMD_NOP             0x00
#define ATA_CMD_READ_PIO        0x20
#define ATA_CMD_READ_PIO_EXT    0x24
#define ATA_CMD_READ_DMA        0xC8
#define ATA_CMD_READ_DMA_EXT    0x25
#define ATA_CMD_WRITE_PIO       0x30
#define ATA_CMD_WRITE_PIO_EXT   0x34
#define ATA_CMD_WRITE_DMA       0xCA
#define ATA_CMD_WRITE_DMA_EXT   0x35
#define ATA_CMD_CACHE_FLUSH     0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET          0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_IDENTIFY        0xEC

/* ATA request */
typedef struct {
#define ATA_PROTO_NODATA        0
#define ATA_PROTO_PIO           1
#define ATA_PROTO_DMA           2
#define ATA_PROTO_PACKET        3
    uint32_t protocol;
#define ATA_CHANNEL_PRIMARY     0
#define ATA_CHANNEL_SECONDARY   1
    uint32_t channel;
#define ATA_DRIVE_MASTER        0
#define ATA_DRIVE_SLAVE         1
    uint32_t drvnum;   /* master or slave? */
    uint16_t seccount; /* num of sectors */
    uint64_t lba;      /* lba */
#define ATA_AMODE_CHS           0
#define ATA_AMODE_LBA28         1
#define ATA_AMODE_LBA48         2
    int32_t  amode;    /* addressing mode */
    uint8_t  cmd;
    uint8_t  *buf;
    uint32_t bufsize;
    uint32_t drqsize;
#define ATA_DIR_READ            0
#define ATA_DIR_WRITE           1
    uint32_t direction;
#define ATA_WMODE_POLLING       0
#define ATA_WMODE_IRQ           1
    uint32_t wmode;    /* waiting mode */
} ata_req_t;

/* drive structure */
typedef struct {
    uint32_t channel;
    uint32_t drvnum;
    uint32_t exists;
#define DRIVE_TYPE_ATA          0
#define DRIVE_TYPE_ATAPI        1
    uint32_t type; /* drive type */
    uint32_t mode; /* addressing mode */
    uint32_t dma;  /* dma supported? */
    uint32_t cylinders;
    uint32_t heads;
    uint32_t sectors;
    uint64_t allsectors;
    uint8_t serial[21];
    uint8_t model[41];
    uint32_t drvreg;
} ata_drive_t;

#endif
