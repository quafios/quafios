/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Window Manager.                             | |
 *        | |  -> PNG format support.                              | |
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gui.h>
#include <api/fs.h>

pixbuf_t *parse_png(char *filename) {

    FILE *file;
    unsigned char sign[8] = {0};
    pixbuf_t *pixbuf; /* the pixel buffer */
    unsigned char *compressed; /* compressed data  */
    int pos = 0; /* pointer for writing data into compressed */
    unsigned char *filt;
    stat_t info;

    /* open the PNG file */
    file = fopen(filename, "r");
    if (!file)
        return NULL;

    /* Read the 8-byte signature */
    fread(sign, sizeof(sign), 1, file);

    if (sign[0] != 0x89 ||
        sign[1] != 'P'  ||
        sign[2] != 'N'  ||
        sign[3] != 'G'  ||
        sign[4] != '\r' ||
        sign[5] != '\n' ||
        sign[6] != 0x1A ||
        sign[7] != '\n') {
        /* invalid PNG file */
        fclose(file);
        return NULL;
    }

    /* allocate pixbuf */
    pixbuf = malloc(sizeof(pixbuf_t));
    pixbuf->buf = NULL;

    /* allocate compressed data buffer */
    stat(filename, &info);
    compressed = malloc((int) info.size);

    /* Read PNG chunks */
    while (1) {

        /* chunk data */
        int len = 0;
        char type[5] = {0};
        int crc;

        /* read chunk length */
        fread(((char *) &len) + 3, 1, 1, file);
        fread(((char *) &len) + 2, 1, 1, file);
        fread(((char *) &len) + 1, 1, 1, file);
        fread(((char *) &len) + 0, 1, 1, file);

        /* read type */
        fread(type, 4, 1, file);

        /* read chunk */
        if (!strcmp(type, "IHDR")) {

            /* header */
            char *header = malloc(len);
            fread(header, len, 1, file);

            pixbuf->width  = (header[ 0]<<24)+(header[ 1]<<16)+
                             (header[ 2]<< 8)+(header[ 3]<< 0);

            pixbuf->height = (header[ 4]<<24)+(header[ 5]<<16)+
                             (header[ 6]<< 8)+(header[ 7]<< 0);

            /* currently we only support files with
             * bits_per_channel = 8 and color type = (true color and alpha).
             */
            if (header[8] != 8 || header[9] != 6) {
                free(header);
                free(pixbuf);
                fclose(file);
                return NULL;
            }

            pixbuf->buf = malloc(pixbuf->height * pixbuf->width*4);

            /* filtered data buffer */
            filt = malloc(pixbuf->height*(pixbuf->width*4+1));

            /* currently, PNG only supports DEFLATE/INFLATE compression
             * method.
             */
            if (header[10] != 0) {
                free(header);
                free(pixbuf);
                fclose(file);
                return NULL;
            }

            /* currently, PNG only supports filter method 0 (adaptive
             * filtering with five basic filter types).
             */
            if (header[11] != 0) {
                free(header);
                free(pixbuf);
                fclose(file);
                return NULL;
            }

            /* currently, we don't support Adam 7 interlace. */
            if (header[12] != 0) {
                free(header);
                free(pixbuf);
                fclose(file);
                return NULL;
            }

            free(header);

        } else if (!strcmp(type, "IDAT")) {

            /* read into compressed */
            fread(&compressed[pos], len, 1, file);

            /* update cur */
            pos += len;

        } else if (!strcmp(type, "IEND")) {

            /* decompress */
            int size = inflate(compressed, pos, filt);

            /* read scanlines */
            int i = 0, j;
            unsigned char *pbuf = (unsigned char *) pixbuf->buf;
            while (i * (pixbuf->width*4 + 1) < size) {

                int filt_x, recon_x;
                int filt_a, recon_a;
                int filt_b, recon_b;
                int filt_c, recon_c;
                int ftype = filt[i * (pixbuf->width*4 + 1)];
                int p, pa, pb, pc, pr;

                /* loop over bytes in the scanline */
                for (j = 0; j < pixbuf->width*4; j++) {

                    /* get Filt(x) */
                    filt_x = filt[i * (pixbuf->width*4 + 1) + 1 + j];

                    /* get Filt(a) and Recon(a) */
                    if (j < 4) {
                        filt_a = recon_a = 0;
                    } else {
                        filt_a  = filt[i * (pixbuf->width*4 + 1) + 1 + j - 4];
                        recon_a = pbuf[i*(pixbuf->width*4) + j - 4];
                    }

                    /* get Filt(b) and Recon(b) */
                    if (!i) {
                        filt_b = recon_b = 0;
                    } else {
                        filt_b  = filt[(i-1)*(pixbuf->width*4 + 1) + 1 + j];
                        recon_b = pbuf[(i-1)*(pixbuf->width*4) + j];
                    }

                    /* get Filt(c) and Recon(c) */
                    if (j < 4 || !i) {
                        filt_c = recon_c = 0;
                    } else {
                        filt_c  = filt[(i-1)*(pixbuf->width*4 + 1) + 1 + j-4];
                        recon_c = pbuf[(i-1)*(pixbuf->width*4) + j-4];
                    }

                    /* apply the anti-filter */
                    switch (ftype) {
                        case 0:
                            recon_x = filt_x;
                            break;
                        case 1:
                            recon_x = filt_x + recon_a;
                            break;
                        case 2:
                            recon_x = filt_x + recon_b;
                            break;
                        case 3:
                            recon_x = filt_x + ((recon_a+recon_b)/2);
                            break;
                        case 4:
                            p = recon_a + recon_b - recon_c;
                            pa = (p-recon_a > 0) ? p-recon_a : recon_a-p;
                            pb = (p-recon_b > 0) ? p-recon_b : recon_b-p;
                            pc = (p-recon_c > 0) ? p-recon_c : recon_c-p;
                            if (pa <= pb && pa <= pc)
                                pr = recon_a;
                            else if (pb <= pc)
                                pr = recon_b;
                            else
                                pr = recon_c;
                            recon_x = filt_x + pr;
                            break;
                        default:
                            break;
                    }

                    /* write the reconstructed byte */
                    pbuf[i*(pixbuf->width*4) + j] = recon_x;

                }

                i++;

            }

            /* color correction */
            for (i = 0; i < pixbuf->width*pixbuf->height; i++) {
                int abgr = pixbuf->buf[i];
                int r = (abgr>> 0) & 0xFF;
                int g = (abgr>> 8) & 0xFF;
                int b = (abgr>>16) & 0xFF;
                int a = (abgr>>24) & 0xFF;
                int argb = (a<<24)|(r<<16)|(g<<8)|(b<<0);
                pixbuf->buf[i] = argb;
            }

            /* deallocate compression buffer */
            free(compressed);

            /* deallocate the filter */
            free(filt);

            /* done */
            fclose(file);
            return pixbuf;

        } else if (type[0] >= 'A' && type[0] <= 'Z') {

            /* unrecognized critical chunk */
            if (pixbuf->buf)
                free(pixbuf->buf);
            free(pixbuf);
            fclose(file);
            return NULL;

        } else {

            /* just ignore */
            char *tmp = malloc(len);
            fread(tmp, len, 1, file);
            free(tmp);

        }

        /* read CRC */
        fread(((char *) &crc) + 3, 1, 1, file);
        fread(((char *) &crc) + 2, 1, 1, file);
        fread(((char *) &crc) + 1, 1, 1, file);
        fread(((char *) &crc) + 0, 1, 1, file);

    }

    /* done */
    fclose(file);
    return NULL;

}
