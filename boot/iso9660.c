/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Boot-Loader.                                | |
 *        | |  -> CDFS driver.                                     | |
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

uint8_t  buffer[2048];
uint32_t *bi_PrimaryVolumeDescriptor = (uint32_t *) 0x7C08;
uint8_t  *boot_type                  = (uint8_t  *) 0x7DF0;
uint8_t  *drive_index                = (uint8_t  *) 0x7DF1;

typedef struct {
    uint8_t  len;
    uint8_t  ext_len;
    uint32_t lba;
    uint32_t lba_big;
    uint32_t size;
    uint32_t size_big;
    uint8_t  date_time[7];
    uint8_t  flags;
    uint8_t  unit_size;
    uint8_t  gap_size;
    uint16_t sequence;
    uint16_t sequence_length;
    uint8_t  id_len;
    uint8_t  id[1];
} __attribute__((packed)) dentry_t ;

int32_t cdread(uint32_t lba, uint16_t memAddr) {

    /* boot type: 0x01 (EL-TORITO)
     *            0x02 (debug/test)
     */

    /* DAP structure */
    struct {
        uint8_t  size;
        uint8_t  unused;
        uint16_t count;
        uint16_t offset;
        uint16_t segment;
        uint64_t lba;
    } __attribute__((packed)) dap;

    int32_t error;

    if (*boot_type == 0x02) {
        uint8_t *src  = (uint8_t *) (lba*2048+0x2000000);
        uint8_t *dest = (uint8_t *) ((int32_t) memAddr);
        int32_t i;
        /* printf("lba: %d, src: %x, dest: %x\n", lba, src, dest); */
        for(i = 0; i < 2048; i++)
            dest[i] = src[i];
        return 2048;
    }


    dap.size    = 0x10;
    dap.unused  = 0x00;
    dap.count   = 0x01;
    dap.offset  = memAddr;
    dap.segment = 0x0000;
    dap.lba     = lba;

    __asm__("int $0x13":"=a"(error)
                       :"S"(&dap), "a"(0x4200), "d"(*drive_index));

    if ((error >> 8) & 0xFF) {
        printf("READ ERROR!\n");
        while(1);
    }

    return 2048;

}

uint32_t isoLookup(char *name, int32_t *size) {

    /* root parameters */
    uint32_t root_lba, root_size, root_sects;

    /* name parameters */
    int32_t  name_len = -1, bytes, i, j;
    uint32_t remaining;

    /* calculate name length: */
    while(name[++name_len]);

    /* load primary volume descriptor: */
    cdread(*bi_PrimaryVolumeDescriptor, (int16_t) buffer);

    /* root directory: */
    root_lba   = *((uint32_t *) &buffer[158]);
    root_size  = *((uint32_t *) &buffer[166]);
    root_sects = (root_size+2047)/2048;

    /* lookup for "name": */
    remaining = root_size;
    while(remaining) {
        /* read next sector: */
        cdread(root_lba++, (int16_t) buffer);

        /* count of bytes in this block: */
        bytes = remaining > 2048 ? 2048 : remaining;

        /* loop on entries: */
        i = 0;
        for (; i < bytes;) {

            /* Get the entry: */
            dentry_t *dentry = (dentry_t *) &buffer[i];

            /* Next entry: */
            i += dentry->len;

            if (!(dentry->len)) {
                /* no more entries in this sector */
                break;
            }

            if (dentry->flags & 0x02) {
                /* not a regular file */
                continue;
            }

            if (dentry->id_len != name_len+2) {
                /* length not matching */
                continue;
            }

            j = 0;
            while(j < name_len && dentry->id[j] == name[j])
                j++;

            if (j != name_len) {
                /* name not matching */
                continue;
            }

            *size = dentry->size;
            return dentry->lba;
        }

        /* update "remaining": */
        remaining -= bytes;

    }

    printf("The file \"(CDROM)/");
    printf(name);
    printf("\" is missing.\n");

    while(1);

}
