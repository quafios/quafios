/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> IBM-compatible VGA device driver.                | |
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
#include <arch/page.h>
#include <sys/error.h>
#include <sys/class.h>
#include <sys/resource.h>
#include <sys/device.h>
#include <sys/mm.h>
#include <sys/bootinfo.h>
#include <video/generic.h>

/* Prototypes: */
uint32_t vga_probe(device_t *, void *);
uint32_t vga_read (device_t *, uint64_t, uint32_t, char *);
uint32_t vga_write(device_t *, uint64_t, uint32_t, char *);
uint32_t vga_ioctl(device_t *, uint32_t, void *);
uint32_t vga_irq  (device_t *, uint32_t);

/* Classes supported: */
static class_t classes[] = {
    {BUS_ISA, BASE_ISA_IBM, SUB_ISA_IBM_VGA, IF_ANY}
};

/* driver_t structure that identifies this driver: */
driver_t vga_driver = {
    /* cls_count: */ sizeof(classes)/sizeof(class_t),
    /* cls:       */ classes,
    /* alias:     */ "vga",
    /* probe:     */ vga_probe,
    /* read:      */ vga_read,
    /* write:     */ vga_write,
    /* ioctl:     */ vga_ioctl,
    /* irq:       */ vga_irq
};

/* ================================================================= */
/*                         VGA Registers                             */
/* ================================================================= */

/* Graphics Registers (index from 0x00 to 0x08): */
#define VGA_GRAPHICS_ADDR       0x3CE
#define VGA_GRAPHICS_READ       0x3CF
#define VGA_GRAPHICS_WRITE      0x3CF
#define VGA_GRAPHICS_COUNT      0x09

/* Sequencer Registers (index from 0x00 to 0x04): */
#define VGA_SEQUENCER_ADDR      0x3C4
#define VGA_SEQUENCER_READ      0x3C5
#define VGA_SEQUENCER_WRITE     0x3C5
#define VGA_SEQUENCER_COUNT     0x05

/* Attribute Registers (index from 0x00 to 0x14): */
#define VGA_ATTRIBUTE_ADDR      0x3C0
#define VGA_ATTRIBUTE_READ      0x3C1
#define VGA_ATTRIBUTE_WRITE     0x3C0
#define VGA_ATTRIBUTE_COUNT     0x15

/* CRT Controller Registers (index from 0x00 to 0x18): */
#define VGA_CRTC_ADDR           0x3D4
#define VGA_CRTC_READ           0x3D5
#define VGA_CRTC_WRITE          0x3D5
#define VGA_CRTC_COUNT          0x19

/* Color Registers (Digital-Analog Converter): */
#define VGA_DAC_STATE           0x3C7
#define VGA_DAC_ADDR_READ       0x3C7
#define VGA_DAC_ADDR_WRITE      0x3C8
#define VGA_DAC_READ            0x3C9
#define VGA_DAC_WRITE           0x3C9

/* External Registers: */
#define VGA_INPUT_STATUS0       0x3C2
#define VGA_MISC_WRITE          0x3C2
#define VGA_FEATURE_READ        0x3CA
#define VGA_MISC_READ           0x3CC
#define VGA_INPUT_STATUS1       0x3DA
#define VGA_FEATURE_WRITE       0x3DA

uint32_t vga_mode;
uint32_t vga_width;
uint32_t vga_height;
uint32_t vga_depth;
uint32_t vga_lfb;
uint32_t vga_scanline;
unsigned char *vga;

/* legacy variables */
extern uint32_t legacy_lfb_enabled;

uint8_t read_misc() {
    return ioread(1, RESOURCE_TYPE_PORT, 0, VGA_MISC_READ);
}

void write_misc(uint8_t val) {
    iowrite(1, RESOURCE_TYPE_PORT, val, 0, VGA_MISC_WRITE);
}

uint8_t read_seq(uint8_t indx) {
    iowrite(1, RESOURCE_TYPE_PORT, indx, 0, VGA_SEQUENCER_ADDR);
    return ioread(1, RESOURCE_TYPE_PORT, 0, VGA_SEQUENCER_READ);
}

void write_seq(uint8_t indx, uint8_t val) {
    iowrite(1, RESOURCE_TYPE_PORT, indx, 0, VGA_SEQUENCER_ADDR);
    iowrite(1, RESOURCE_TYPE_PORT, val, 0, VGA_SEQUENCER_WRITE);
}

uint8_t read_crtc(uint8_t indx) {
    iowrite(1, RESOURCE_TYPE_PORT, indx, 0, VGA_CRTC_ADDR);
    return ioread(1, RESOURCE_TYPE_PORT, 0, VGA_CRTC_READ);
}

void write_crtc(uint8_t indx, uint8_t val) {
    iowrite(1, RESOURCE_TYPE_PORT, indx, 0, VGA_CRTC_ADDR);
    iowrite(1, RESOURCE_TYPE_PORT, val, 0, VGA_CRTC_WRITE);
}

uint8_t read_gx(uint8_t indx) {
    iowrite(1, RESOURCE_TYPE_PORT, indx, 0, VGA_GRAPHICS_ADDR);
    return ioread(1, RESOURCE_TYPE_PORT, 0, VGA_GRAPHICS_READ);
}

void write_gx(uint8_t indx, uint8_t val) {
    iowrite(1, RESOURCE_TYPE_PORT, indx, 0, VGA_GRAPHICS_ADDR);
    iowrite(1, RESOURCE_TYPE_PORT, val, 0, VGA_GRAPHICS_WRITE);
}

uint8_t read_attr(uint8_t indx) {
    /* notice that this blanks the display */
    ioread(1, RESOURCE_TYPE_PORT, 0, VGA_INPUT_STATUS1);
    iowrite(1, RESOURCE_TYPE_PORT, indx, 0, VGA_ATTRIBUTE_ADDR);
    return ioread(1, RESOURCE_TYPE_PORT, 0, VGA_ATTRIBUTE_READ);
}

void write_attr(uint8_t indx, uint8_t val) {
    /* notice that this blanks the display */
    ioread(1, RESOURCE_TYPE_PORT, 0, VGA_INPUT_STATUS1);
    iowrite(1, RESOURCE_TYPE_PORT, indx, 0, VGA_ATTRIBUTE_ADDR);
    iowrite(1, RESOURCE_TYPE_PORT, val, 0, VGA_ATTRIBUTE_WRITE);
}

void set_PAS() {
    /* this locks 16-color palette and unblanks display. */
    ioread(1, RESOURCE_TYPE_PORT, 0, VGA_INPUT_STATUS1);
    iowrite(1, RESOURCE_TYPE_PORT, 0x20, 0, VGA_ATTRIBUTE_ADDR);
}

void write_dac(uint8_t indx, uint8_t red, uint8_t green, uint8_t blue) {
    iowrite(1, RESOURCE_TYPE_PORT, indx, 0, VGA_DAC_ADDR_WRITE);
    iowrite(1, RESOURCE_TYPE_PORT, red, 0, VGA_DAC_WRITE);
    iowrite(1, RESOURCE_TYPE_PORT, green, 0, VGA_DAC_WRITE);
    iowrite(1, RESOURCE_TYPE_PORT, blue, 0, VGA_DAC_WRITE);
}

void vga_regs_unlock() {
    /* called on initialization */
    write_misc(0x67);
    write_crtc(0x03, read_crtc(0x03) | 0x80);
    write_crtc(0x11, read_crtc(0x11) & 0x7F);
}

/* ================================================================= */
/*                            VGA Modes                              */
/* ================================================================= */

uint8_t text_80x25x16[] = {
    /* MISC register: */
    0x67,
    /* SEQ registers: */
    0x03, 0x00, 0x03, 0x00, 0x02,
    /* CRTC registers: */
    0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
    0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x50,
    0x9C, 0x0E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
    0xFF,
    /* GX registers: */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
    0xFF,
    /* ATTR registers: */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x0C, 0x00, 0x0F, 0x08, 0x00,
};

/* graphix 320x200 with 256 colors */
uint8_t gx_320x200x256[] = {
    /* MISC register: */
    0x63,
    /* SEQ registers: */
    0x03, 0x01, 0x0F, 0x00, 0x0E,
    /* CRTC registers: */
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
    0x00, 0x41, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x50,
    0x9C, 0x8E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
    0xFF,
    /* GX registers: */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x00,
    0xFF,
    /* ATTR registers: */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x41, 0x00, 0x0F, 0x00, 0x00,
};

/* graphics 640x480 with 16-bit colors, planar. */
uint8_t gx_640x480x16[] = {
    /* MISC register: */
    0xE3,
    /* SEQ registers: */
    0x03, 0x01, 0x0F, 0x00, 0x06,
    /* CRTC registers: */
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
    0x00, 0x40, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x50,
    0xEA, 0x8C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
    0xFF,
    /* GX registers: */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00,
    0xFF,
    /* ATTR registers: */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x01, 0x00, 0x0F, 0x00, 0x00,
};

void set_mode(uint8_t *mode) {

    /* index in the mode array */
    int32_t i = 0, j;

    /* unlock crtc registers */
    vga_regs_unlock();

    /* misc: */
    write_misc(mode[i++]);

    /* seq: */
    for (j = 0; j < VGA_SEQUENCER_COUNT; j++)
        write_seq(j, mode[i++]);

    /* crtc: */
    for (j = 0; j < VGA_CRTC_COUNT; j++)
        write_crtc(j, mode[i++]);

    /* gx: */
    for (j = 0; j < VGA_GRAPHICS_COUNT; j++)
        write_gx(j, mode[i++]);

    /* attr: */
    for (j = 0; j < VGA_ATTRIBUTE_COUNT; j++)
        write_attr(j, mode[i++]); /* NOTE: this blanks the display */

    /* unblank the display */
    set_PAS();

}

/* ================================================================= */
/*                          256 Color Pallette                       */
/* ================================================================= */

/* CREDIT: I got the rgb values of colors from C++ code
 *         of tauron project (by Jeff Morgan).
 */
uint8_t color_palette[] = {
     0,  0,  0,
     0,  0, 42,
     0, 42,  0,
     0, 42, 42,
    42,  0,  0,
    42,  0, 42,
    42, 21,  0,
    42, 42, 42,
    21, 21, 21,
    21, 21, 63,
    21, 63, 21,
    21, 63, 63,
    63, 21, 21,
    63, 21, 63,
    63, 63, 21,
    63, 63, 63,
     0,  0,  0,
     5,  5,  5,
     8,  8,  8,
    11, 11, 11,
    14, 14, 14,
    17, 17, 17,
    20, 20, 20,
    24, 24, 24,
    28, 28, 28,
    32, 32, 32,
    36, 36, 36,
    40, 40, 40,
    45, 45, 45,
    50, 50, 50,
    56, 56, 56,
    63, 63, 63,
     0,  0, 63,
    16,  0, 63,
    31,  0, 63,
    47,  0, 63,
    63,  0, 63,
    63,  0, 47,
    63,  0, 31,
    63,  0, 16,
    63,  0,  0,
    63, 16,  0,
    63, 31,  0,
    63, 47,  0,
    63, 63,  0,
    47, 63,  0,
    31, 63,  0,
    16, 63,  0,
     0, 63,  0,
     0, 63, 16,
     0, 63, 31,
     0, 63, 47,
     0, 63, 63,
     0, 47, 63,
     0, 31, 63,
     0, 16, 63,
    31, 31, 63,
    39, 31, 63,
    47, 31, 63,
    55, 31, 63,
    63, 31, 63,
    63, 31, 55,
    63, 31, 47,
    63, 31, 39,
    63, 31, 31,
    63, 39, 31,
    63, 47, 31,
    63, 55, 31,
    63, 63, 31,
    55, 63, 31,
    47, 63, 31,
    39, 63, 31,
    31, 63, 31,
    31, 63, 39,
    31, 63, 47,
    31, 63, 55,
    31, 63, 63,
    31, 55, 63,
    31, 47, 63,
    31, 39, 63,
    45, 45, 63,
    49, 45, 63,
    54, 45, 63,
    58, 45, 63,
    63, 45, 63,
    63, 45, 58,
    63, 45, 54,
    63, 45, 49,
    63, 45, 45,
    63, 49, 45,
    63, 54, 45,
    63, 58, 45,
    63, 63, 45,
    58, 63, 45,
    54, 63, 45,
    49, 63, 45,
    45, 63, 45,
    45, 63, 49,
    45, 63, 54,
    45, 63, 58,
    45, 63, 63,
    45, 58, 63,
    45, 54, 63,
    45, 49, 63,
     0,  0, 28,
     7,  0, 28,
    14,  0, 28,
    21,  0, 28,
    28,  0, 28,
    28,  0, 21,
    28,  0, 14,
    28,  0,  7,
    28,  0,  0,
    28,  7,  0,
    28, 14,  0,
    28, 21,  0,
    28, 28,  0,
    21, 28,  0,
    14, 28,  0,
     7, 28,  0,
     0, 28,  0,
     0, 28,  7,
     0, 28, 14,
     0, 28, 21,
     0, 28, 28,
     0, 21, 28,
     0, 14, 28,
     0,  7, 28,
    14, 14, 28,
    17, 14, 28,
    21, 14, 28,
    24, 14, 28,
    28, 14, 28,
    28, 14, 24,
    28, 14, 21,
    28, 14, 17,
    28, 14, 14,
    28, 17, 14,
    28, 21, 14,
    28, 24, 14,
    28, 28, 14,
    24, 28, 14,
    21, 28, 14,
    17, 28, 14,
    14, 28, 14,
    14, 28, 17,
    14, 28, 21,
    14, 28, 24,
    14, 28, 28,
    14, 24, 28,
    14, 21, 28,
    14, 17, 28,
    20, 20, 28,
    22, 20, 28,
    24, 20, 28,
    26, 20, 28,
    28, 20, 28,
    28, 20, 26,
    28, 20, 24,
    28, 20, 22,
    28, 20, 20,
    28, 22, 20,
    28, 24, 20,
    28, 26, 20,
    28, 28, 20,
    26, 28, 20,
    24, 28, 20,
    22, 28, 20,
    20, 28, 20,
    20, 28, 22,
    20, 28, 24,
    20, 28, 26,
    20, 28, 28,
    20, 26, 28,
    20, 24, 28,
    20, 22, 28,
     0,  0, 16,
     4,  0, 16,
     8,  0, 16,
    12,  0, 16,
    16,  0, 16,
    16,  0, 12,
    16,  0,  8,
    16,  0,  4,
    16,  0,  0,
    16,  4,  0,
    16,  8,  0,
    16, 12,  0,
    16, 16,  0,
    12, 16,  0,
     8, 16,  0,
     4, 16,  0,
     0, 16,  0,
     0, 16,  4,
     0, 16,  8,
     0, 16, 12,
     0, 16, 16,
     0, 12, 16,
     0,  8, 16,
     0,  4, 16,
     8,  8, 16,
    10,  8, 16,
    12,  8, 16,
    14,  8, 16,
    16,  8, 16,
    16,  8, 14,
    16,  8, 12,
    16,  8, 10,
    16,  8,  8,
    16, 10,  8,
    16, 12,  8,
    16, 14,  8,
    16, 16,  8,
    14, 16,  8,
    12, 16,  8,
    10, 16,  8,
     8, 16,  8,
     8, 16, 10,
     8, 16, 12,
     8, 16, 14,
     8, 16, 16,
     8, 14, 16,
     8, 12, 16,
     8, 10, 16,
    11, 11, 16,
    12, 11, 16,
    13, 11, 16,
    15, 11, 16,
    16, 11, 16,
    16, 11, 15,
    16, 11, 13,
    16, 11, 12,
    16, 11, 11,
    16, 12, 11,
    16, 13, 11,
    16, 15, 11,
    16, 16, 11,
    15, 16, 11,
    13, 16, 11,
    12, 16, 11,
    11, 16, 11,
    11, 16, 12,
    11, 16, 13,
    11, 16, 15,
    11, 16, 16,
    11, 15, 16,
    11, 13, 16,
    11, 12, 16,
     0,  0,  0,
     0,  0,  0,
     0,  0,  0,
     0,  0,  0,
     0,  0,  0,
     0,  0,  0,
     0,  0,  0,
    63, 63, 63
};

void set_palette(uint8_t *pallette) {

    /* index */
    int32_t i;

    /* loop on the pallette */
    for (i = 0; i < 256; i++)
        write_dac(i, pallette[i*3+0], pallette[i*3+1], pallette[i*3+2]);

}

void vga_plot(vga_plot_t *plot) {

    int32_t x, y;
    for (y = 0; y < plot->height; y++)
        for (x = 0; x < plot->width; x++) {

            int dx = x+plot->x;
            int dy = y+plot->y;

            int32_t src_off  = (y*plot->width + x)*4;
            int32_t dest_off = dy*vga_scanline+dx*((vga_depth+1)/8);

            if (dx >= vga_width || dy >= vga_height)
                continue;

            vga[dest_off + 0] = plot->buf[src_off + 0];
            vga[dest_off + 1] = plot->buf[src_off + 1];
            vga[dest_off + 2] = plot->buf[src_off + 2];

        }

}

/* ================================================================= */
/*                            Interface                              */
/* ================================================================= */

uint32_t vga_probe(device_t *dev, void *config) {

    extern bootinfo_t *bootinfo;
    extern uint8_t *legacy_vga_fbuf;
    int vga_size, vga_pages, i;
    unsigned char *vga_white;

    /* inform the user that VGA is being initialized. */
    printk("VGA Device Driver is loading...\n");

    /* get vga mode from bootinfo structure */
    vga_mode = bootinfo->vga_mode;

    /* add to devfs */
    devfs_reg("vga", dev->devid);

    /* do nothing if graphics is disabled */
    if (vga_mode == 0)
        return ESUCCESS;

    /* If no vesa graphics mode is already applied, apply
     * the 320x200x256 mode.
     */
    if (bootinfo->vga_width == 0) {
        /* set VGA mode: */
        vga_width  = 320;
        vga_height = 200;
        vga_depth  = 8;
        vga_lfb    = 0xA0000;
        vga_scanline = vga_width;
        set_mode(gx_320x200x256);

        /* write the color palette to DAC memory: */
        set_palette(color_palette);
    } else {
        /* read VESA mode parameters */
        vga_width    = bootinfo->vga_width;
        vga_height   = bootinfo->vga_height;
        vga_depth    = bootinfo->vga_depth;
        vga_lfb      = bootinfo->vga_phys;
        vga_scanline = bootinfo->vga_scanline;
    }

    /* set buffer to legacy framebuffer */
    vga = legacy_vga_fbuf;

    /* done: */
    return ESUCCESS;

}

uint32_t vga_read(device_t *dev, uint64_t off, uint32_t size, char *buff) {

}

uint32_t vga_write(device_t *dev, uint64_t off, uint32_t size, char *buff) {

    /* If the legacy driver already has access to LFB, this
     * means that the system is working in text mode.
     */
    if (legacy_lfb_enabled)
        return 0;

    __asm__("rep movsl;"::"S"(buff), "D"(vga), "c"(size/4));
}

uint32_t vga_ioctl(device_t *dev, uint32_t cmd, void *data) {
    switch (cmd) {
        case VGA_GET_MODE:
            *((uint32_t *) data) = vga_mode;
            break;
        case VGA_GET_WIDTH:
            *((uint32_t *) data) = vga_width;
            break;
        case VGA_GET_HEIGHT:
            *((uint32_t *) data) = vga_height;
            break;
        case VGA_GET_DEPTH:
            *((uint32_t *) data) = vga_depth;
            break;
        case VGA_GET_SCANLINE:
            *((uint32_t *) data) = vga_scanline;
            break;
        case VGA_PLOT:
            /* If the legacy driver already has access to LFB, this
             * means that the system is working in text mode.
             * In this case, vga_plot() won't run.
             */
            if (!legacy_lfb_enabled)
                vga_plot(data);
            break;
        default:
            break;
    }
    return ESUCCESS;
}

uint32_t vga_irq(device_t *dev, uint32_t irqn) {
    return ESUCCESS;
}
