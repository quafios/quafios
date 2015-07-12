/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Boot-Loader.                                | |
 *        | |  -> VGA routines.                                    | |
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
#include <sys/bootinfo.h>

#define ENABLE_GRAPHICS 0

uint8_t *vga   = (uint8_t *) 0xB8000;
uint8_t attrib = 0x0F;

/* FIXME: all interrupt calls should be preceded with PUSHA and
 *        should be followed by POPA.
 */

uint32_t getX() {

    uint16_t pos;
    int32_t x, y;

    /* Get cursor position */
    __asm__("int $0x10":"=d"(pos):"a"(0x0300), "b"(0));

    /* Set x & y: */
    x = (pos>>0) & 0xFF; /* column (DL) */
    y = (pos>>8) & 0xFF; /* row    (DH) */

    /* return X */
    return x;

}

uint32_t getY() {

    uint16_t pos;
    int32_t x, y;

    /* Get cursor position */
    __asm__("int $0x10":"=d"(pos):"a"(0x0300), "b"(0));

    /* Set x & y: */
    x = (pos>>0) & 0xFF; /* column (DL) */
    y = (pos>>8) & 0xFF; /* row    (DH) */

    /* return Y */
    return y;

}

void cursor(int32_t x, int32_t y) {

    uint16_t pos = 0;

    /* Get x & y: */
    pos |= x<<0;  /* column (DL) */
    pos |= y<<8;  /* row    (DH) */

    /* Set cursor position */
    __asm__("int $0x10"::"a"(0x0200), "b"(0), "d"(pos));

}

void cls() {
    int32_t i = 0;
    while(i < 80*25*2) {
        vga[i++] = ' ';
        vga[i++] = attrib;
    }
    cursor(0, 0);
}

void scroll() {
    uint32_t i;
    for (i=0; i< 80 * 24 * 2; i++)
        vga[i] = vga[i + 80 * 2];

    /* clean last row: */
    for (; i < 80 * 25 * 2; i+=2)
        vga[i] = ' ';
}

void putc(char chr) {

    int32_t new_x, new_y;
    int32_t x, y;

    if (chr == '\n') {
        do putc(' '); while(getX());
        return;
    }

    x = getX();
    y = getY();

    vga[(y*80 + x)*2 + 0] = chr;
    vga[(y*80 + x)*2 + 1] = attrib;

    if (x != 79) {
        new_x = x+1;
        new_y = y;
    } else {
        new_x = 0;
        if (y != 24) {
            new_y = y+1;
        } else {
            scroll();
            new_y = y;
        }
    }

    cursor(new_x, new_y);
}

void printDec(uint32_t value) {
    uint32_t n = value / 10;
    int32_t r = value % 10;
    if (value >= 10) printDec(n);
    putc(r+'0');
}

void printHex(uint32_t value) {
    int32_t i;
    for(i = 7; i >= 0; i--)
        putc("0123456789ABCDEF"[(value>> (i*4)) & 0x0F]);
}

void printf(char *format, ...) {

    uint32_t arg_id = 0, i;
    int32_t  *addr = (int32_t *) &format;

    for (i = 0; format[i] != 0; i++)
        if (format[i] == '%')
            switch (format[++i]) {
                case 'c':
                    putc(addr[++arg_id]);
                    break;

                case 'd':
                    printDec(addr[++arg_id]);
                    break;

                case 'x':
                    printHex(addr[++arg_id]);
                    break;

                case 's':
                    printf((char *) addr[++arg_id]);
                    break;

                default:
                    break;
            }
        else
            putc(format[i]);

}

typedef struct {
   char     VbeSignature[4];
   uint16_t VbeVersion;
   uint16_t OemStringPtr[2];
   uint8_t  Capabilities[4];
   uint16_t VideoModePtr[2];
   uint16_t TotalMemory;
} __attribute__((packed)) VbeInfoBlock_t;

typedef struct {
    uint16_t attributes;
    uint8_t  winA,winB;
    uint16_t granularity;
    uint16_t winsize;
    uint16_t segmentA, segmentB;
    uint32_t realFctPtr;
    uint16_t pitch;

    uint16_t Xres, Yres;
    uint8_t  Wchar, Ychar, planes, bpp, banks;
    uint8_t  memory_model, bank_size, image_pages;
    uint8_t  reserved0;

    uint8_t  red_mask, red_position;
    uint8_t  green_mask, green_position;
    uint8_t  blue_mask, blue_position;
    uint8_t  rsv_mask, rsv_position;
    uint8_t  directcolor_attributes;

    uint32_t physbase;
    uint32_t reserved1;
    uint16_t reserved2;
} ModeInfoBlock_t;

uint8_t vbeinfo_arr[512];
VbeInfoBlock_t *vbeinfo;

uint8_t modeinfo_arr[256];
ModeInfoBlock_t *modeinfo;

uint16_t get_mode(int segment, int offset, int i) {
    uint16_t ret;
    __asm__("push %%ds;        \n\
             push %%ax;        \n\
             pop  %%ds;        \n\
             mov  (%%si), %%ax \n\
             pop  %%ds;        \n":"=a"(ret)
                                  :"a"(segment), "S"(offset + i*2));
    return ret;
}

uint32_t find_mode(uint32_t *width,
                   uint32_t *height,
                   uint32_t depth,
                   uint32_t *phy,
                   uint32_t *scanline) {

    int32_t i;
    uint32_t mode, err, ret = 0xFFFF;
    *width = 800;
    *height = 600;

    /* init vbeinfo and modeinfo pointers */
    vbeinfo = (VbeInfoBlock_t *) vbeinfo_arr;
    modeinfo = (ModeInfoBlock_t *) modeinfo_arr;

    /* initialize the signature */
    vbeinfo->VbeSignature[0] = 'V';
    vbeinfo->VbeSignature[1] = 'B';
    vbeinfo->VbeSignature[2] = 'E';
    vbeinfo->VbeSignature[3] = '2';

    /* read vbe info block */
    __asm__("int $0x10":"=a"(err):"a"(0x4F00), "D"(vbeinfo));

    /* error? */
    if (err != 0x004F)
        return 0xFFFF;

    /* get all modes */
    for (i = 0; (mode=get_mode(vbeinfo->VideoModePtr[1],
                               vbeinfo->VideoModePtr[0],i))!=0xFFFF;i++) {

        /* read modeinfo */
        __asm__("int $0x10"::"a"(0x4F01), "c"(mode), "D"(modeinfo));

        printf("mode: %x, ", mode);
        printf("res: %x, ", (modeinfo->Xres<<16) + modeinfo->Yres);
        printf("bpp: %x, ", modeinfo->bpp);
        printf("model: %x, ", modeinfo->memory_model);
        printf("attr: %x\n", modeinfo->attributes);

        /* must be a graphics mode with linear frame buffer support */
        if ((modeinfo->attributes & 0x90) != 0x90)
            continue;

        /* must be a packed pixel or direct color mode */
        if (modeinfo->memory_model != 4 && modeinfo->memory_model != 6)
            continue;

        /* Is this the desired mode? */
        if (modeinfo->Xres * modeinfo->Yres == (*width)*(*height) &&
             modeinfo->bpp  == depth) {
            *width =  modeinfo->Xres;
            *height = modeinfo->Yres;
            *phy = modeinfo->physbase;
            *scanline = modeinfo->pitch;
            ret = mode;
        }
    }

    return ret;

}

void set_resolution() {

    uint32_t mode;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t phy;
    uint32_t scanline;
    extern bootinfo_t *bootinfo;

    /* find mode */
    if ((!ENABLE_GRAPHICS) ||(
        (mode = find_mode(&width, &height, depth=32, &phy, &scanline)) ==
        0xFFFF &&
        (mode = find_mode(&width, &height, depth=24, &phy, &scanline)) ==
        0xFFFF)) {
        printf("Warning: VGA graphics mode is disabled or not supported\n");
        bootinfo->vga_mode = 0; /* console mode */
        return;
    }

    printf("mode:     %x\n", mode);
    printf("width:    %x\n", width);
    printf("height:   %x\n", height);
    printf("depth:    %x\n", depth);
    printf("physic:   %x\n", phy);
    printf("scanline: %x\n", scanline);

    bootinfo->vga_width    = width;
    bootinfo->vga_height   = height;
    bootinfo->vga_depth    = depth;
    bootinfo->vga_phys     = phy;
    bootinfo->vga_scanline = scanline;
    bootinfo->vga_mode     = 1; /* graphics mode */

    /* set mode */
    __asm__("int $0x10"::"a"(0x4F02), "b"(mode | 0x4000));

}

void video_mode(uint8_t mode) {
    __asm__("int $0x10"::"a"(mode));
}
