/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Boot-Loader.                                | |
 *        | |  -> Decompression utility.                           | |
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
#include <sys/bootinfo.h>

extern bootinfo_t *bootinfo;

/****************************************************************************/
/*                                Streams                                   */
/****************************************************************************/

uint8_t  in[2048];
uint32_t inp = 0;    /* in pointer. */
uint8_t  inb = 1;    /* in bit. */
uint32_t sector = 0;
uint32_t rem = 0;

uint8_t  *out;
int32_t  outp = 0;   /* out pointer. */

uint8_t print_counter = 0;

void streamInit(char *file) {

    /* Get first block of the file: */
    sector = isoLookup(file, &rem);

    /* print progress: */
    printf("Decompressing \"");
    printf(file);
    printf("\"...");

    /* load first sector: */
    if (!rem) {
        printf("\nZERO SIZE FILE!\n");
        while(1);
    }
    cdread(sector++, in);

    /* output: */
    out = (uint8_t *) ((int32_t) bootinfo->res[BI_RAMDISK].base);

}

uint8_t getNextBit() {

    uint8_t ret;

    if (!rem) {
        printf("\nEOF!\n");
        while(1);
    }

    ret = in[inp] & inb;
    if (inb == 0x80) {
        inp += (inb = 1);
        rem--;
    } else {
        inb <<= 1;
    }

    if (inp == 2048) {
        inp = 0;
        cdread(sector++, in);
        print_counter++;
        if (print_counter == 20) {
            print_counter = 0;
            printf(".");
        }
        /*while(1); */
    }

    return ret ? 1 : 0;
}

uint32_t getNextN(int32_t N) {

    uint32_t ret = 0, i = 0;

    while (i < N)
        ret |= (getNextBit() << i++);

    return ret;

}

uint8_t getNextByte() {
    return getNextN(8);
}


uint16_t getNextWord() {
    return getNextN(16);
}

uint32_t getNextLong() {
    return getNextN(32);
}

void seekNextByte() {
    while (inb != 1)
        getNextBit();
}

void putByte(uint8_t b) {
    out[outp++] = b;
    /*printf("%c", b); */
}

void putRange(int32_t backward, int32_t size) {

    while(size--) {

        putByte(out[outp-backward]);

    }

}

/****************************************************************************/
/*                                  Trees                                   */
/****************************************************************************/

/* In DEFLATE, maximum code length is 15.
 * from the prespective of binary trees, this
 * means that maximum level count = 15 + 1 = 16
 * therefore maximum count of elements = 2^16-1.
 */

typedef uint16_t node;

#define MAX_BITS        15
#define TREE_SIZE       (64*1024)   /* 2^16 */
#define TREE_BYTES      (TREE_SIZE*sizeof(node))
#define TREE_BASE(N)    ((node *)(0x100000 + TREE_BYTES*N))
#define LEAF_MASK       0x8000
#define VALUE_MASK      0x7FFF

int32_t initTree(node *tree) {
    int32_t i = 0;
    while(i < TREE_SIZE)
        tree[i++] = 0;
}

int32_t getRoot(node *tree) {
    return 0;
}

int32_t getLeft(node *tree, int32_t i) {
    /* next zero.. */
    return i*2 + 1;
}

int32_t getRight(node *tree, int32_t i) {
    /* next one.. */
    return i*2 + 2;
}

int32_t isLeaf(node *tree, int32_t i) {
    return (tree[i] & LEAF_MASK ? 1 : 0);
}

void unLeaf(node *tree, int32_t i) {
    tree[i] &= ~LEAF_MASK;
}

void setLeaf(node *tree, int32_t i) {
    tree[i] |= LEAF_MASK;
}

node getValue(node *tree, int32_t i) {
    return tree[i] & VALUE_MASK;
}

void setValue(node *tree, int32_t i, int16_t val) {
    tree[i] = (tree[i] & LEAF_MASK) | (val & VALUE_MASK);
}

/****************************************************************************/
/*                                 Huffman                                  */
/****************************************************************************/

/* We have three types of elements: literal/length, distances,
 * and code length sequences.
 * every type is encoded/decoded using a type-specific huffman
 * tree.
 */

/* Available Trees: */
/* ---------------- */
int32_t  dynamic;   /* 0: fixed, 1: dynamic */

node *fixedLitTree    = TREE_BASE(0);
node *fixedDistTree   = TREE_BASE(1);

node *codeLenTree     = TREE_BASE(2);
node *dynamicLitTree  = TREE_BASE(3);
node *dynamicDistTree = TREE_BASE(4);

/* look up table for literals/length elements: */
int32_t litLookup[][2] = {
    /* 257 */ {0,   3},
    /* 258 */ {0,   4},
    /* 259 */ {0,   5},
    /* 260 */ {0,   6},
    /* 261 */ {0,   7},
    /* 262 */ {0,   8},
    /* 263 */ {0,   9},
    /* 264 */ {0,  10},
    /* 265 */ {1,  11},
    /* 266 */ {1,  13},
    /* 267 */ {1,  15},
    /* 268 */ {1,  17},
    /* 269 */ {2,  19},
    /* 270 */ {2,  23},
    /* 271 */ {2,  27},
    /* 272 */ {2,  31},
    /* 273 */ {3,  35},
    /* 274 */ {3,  43},
    /* 275 */ {3,  51},
    /* 276 */ {3,  59},
    /* 277 */ {4,  67},
    /* 278 */ {4,  83},
    /* 279 */ {4,  99},
    /* 280 */ {4, 115},
    /* 281 */ {5, 131},
    /* 282 */ {5, 163},
    /* 283 */ {5, 195},
    /* 284 */ {5, 227},
    /* 285 */ {0, 258}
};

/* look up table for distance elements: */
int32_t distLookup[][2] = {
    /*  0 */ { 0,     1},
    /*  1 */ { 0,     2},
    /*  2 */ { 0,     3},
    /*  3 */ { 0,     4},
    /*  4 */ { 1,     5},
    /*  5 */ { 1,     7},
    /*  6 */ { 2,     9},
    /*  7 */ { 2,    13},
    /*  8 */ { 3,    17},
    /*  9 */ { 3,    25},
    /* 10 */ { 4,    33},
    /* 11 */ { 4,    49},
    /* 12 */ { 5,    65},
    /* 13 */ { 5,    97},
    /* 14 */ { 6,   129},
    /* 15 */ { 6,   193},
    /* 16 */ { 7,   257},
    /* 17 */ { 7,   385},
    /* 18 */ { 8,   513},
    /* 19 */ { 8,   769},
    /* 20 */ { 9,  1025},
    /* 21 */ { 9,  1537},
    /* 22 */ {10,  2049},
    /* 23 */ {10,  3073},
    /* 24 */ {11,  4097},
    /* 25 */ {11,  6145},
    /* 26 */ {12,  8193},
    /* 27 */ {12, 12289},
    /* 28 */ {13, 16385},
    /* 29 */ {13, 24577}
};

int32_t lenOrder[] = {
    16, 17, 18, 0,  8, 7,  9, 6,
    10,  5, 11, 4, 12, 3, 13, 2,
    14,  1, 15
};

/* Read next encoded element:  */
/* --------------------------- */

int32_t readUsingTree(node *tree) {

    /* read next bits from the stream
     * using the huffman tree specified by "tree".
     */
    int32_t curNode = getRoot(tree);

    while(1) {
        int32_t bit = getNextBit();

        if (bit == 0) {
            curNode = getLeft(tree, curNode);
        } else {
            curNode = getRight(tree, curNode);
        }

        if (isLeaf(tree, curNode)) {
            return getValue(tree, curNode);
        }
    }

}


int32_t readLiteral() {

    /* Return Value:
     * 0-255: Normal Literal.
     * 256:   End of Block.
     * 256 + N: Length N.
     */
    int32_t lit;

    if (!dynamic) {
        lit = readUsingTree(fixedLitTree);
    } else {
        lit = readUsingTree(dynamicLitTree);
    }

    if (lit <= 256) {
        /* literal */
        return lit;
    } else {
        /* length */
        int32_t nextBits = getNextN(litLookup[lit-257][0]);
        return 256 + litLookup[lit-257][1] + nextBits;
    }

}

int32_t readDistance() {

    int32_t val, bits;

    if (!dynamic) {
        val = readUsingTree(fixedDistTree);
    } else {
        val = readUsingTree(dynamicDistTree);
    }

    bits = distLookup[val][0];
    return getNextN(bits) + distLookup[val][1];

}

int32_t readCodeLen() {
    return readUsingTree(codeLenTree);
}

/* Generate a huffman trees:  */
/* -------------------------- */

void genHuffmanTree(int32_t *len, int32_t max_code, node *tree) {

    /* Generate a huffman tree using a sequence of
     * code lengths.
     * in RFC1951, this algorithm is called "Huffman
     * code construction algorithm".
     */

    int32_t i, bits, n, code, node, bit;
    int32_t bl_count[MAX_BITS+1] = {0};
    int32_t next_code[MAX_BITS+1] = {0};

    /* 1: Count the number of codes for each code length:  */
    /* --------------------------------------------------- */
    for (n = 0;  n <= max_code; n++)
        bl_count[len[n]]++;

    /* 2: Find the numerical value for smallest code for each length:  */
    /* --------------------------------------------------------------- */
    code = 0;
    bl_count[0] = 0;
    for (bits = 1; bits <= MAX_BITS; bits++) {
        code = (code + bl_count[bits-1]) << 1;
        next_code[bits] = code;
    }

    /* 3: Assign numerical values to all codes:  */
    /* ----------------------------------------- */
    for (n = 0;  n <= max_code; n++) {
        bits = len[n]; /* count of bits. */
        if (bits != 0) {
            code = next_code[bits];

            /* "code" represents the series
             * of bits that will be used to
             * represent the alphabet "n".
             */

            node = getRoot(tree);

            for (i = bits-1; i >= 0; i--) {
                bit = (code>>i) & 1;
                unLeaf(tree, node);

                if (bit == 0) {
                    node = getLeft(tree,node);
                } else {
                    node = getRight(tree,node);
                }

            }

            setValue(tree, node, n);
            setLeaf(tree, node);

            next_code[bits]++;
        }
    }

}

void genFixedTrees() {

    int32_t len[288], i;

    /* create length table for the fixed
     * liter/length code tree:
     */
    for (i = 0; i <= 143; i++)
        len[i] = 8;

    for (i = 144; i <= 255; i++)
        len[i] = 9;

    for (i = 256; i <= 279; i++)
        len[i] = 7;

    for (i = 280; i <= 287; i++)
        len[i] = 8;

    /* generate the tree: */
    genHuffmanTree(len, 287, fixedLitTree);

    /* create length table for the fixed distance code tree: */
    for (i = 0; i <= 31; i++)
        len[i] = 5;

    /* generate the tree: */
    genHuffmanTree(len, 31, fixedDistTree);

}

void genDynamicTrees() {

    int32_t len[288+32] = {0}, i;

    /* read sizes of length tables: */
    int32_t litLen  = getNextN(5) /* HLIT  */ + 257;
    int32_t distLen = getNextN(5) /* HDIST */ + 1;
    int32_t codeLen = getNextN(4) /* HCLEN */ + 4;

    /* Read length table for code length tree: */
    for (i = 0; i < codeLen; i++)
        len[lenOrder[i]] = getNextN(3);

    /* Generate the code length huffman tree: */
    genHuffmanTree(len, 18, codeLenTree);

    /* Now read lengths for literal/length alphabet and
     * distance alphabet using the just-generated codeLenTree
     */
    i = 0;
    while (i < litLen + distLen) {

        /* read next number */
        int32_t val = readCodeLen();
        int32_t repeat = 0;

        /* interpret "val": */
        if (val <= 15) {
            /* just a length... */
            len[i++] = val;
            continue;
        } else if (val == 16) {
            val    = i == 0 ? 0 : len[i-1];
            repeat = getNextN(2) + 3;
        } else if (val == 17) {
            val    = 0;
            repeat = getNextN(3) + 3;
        } else {
            val    = 0;
            repeat = getNextN(7) + 11;
        }

        /* Loop: */
        while (repeat--)
            len[i++] = val;
    }

    /* Generate literals/lengths huffman tree: */
    genHuffmanTree(len, litLen-1, dynamicLitTree);

    /* Generate distance tree: */
    genHuffmanTree(&len[litLen], distLen-1, dynamicDistTree);

}

/****************************************************************************/
/*                                   LZ77                                   */
/****************************************************************************/

int32_t readNextElement() {

    /* a file coded in LZ77 consists of a series of elements
     * every element maybe a literal, or a pair of length/distance.
     *
     * in DEFLATE algorithm, elements are of two types,
     * literal/length, and distance.
     *
     * the encoded stream is a series of literal/length elements,
     * if the literal/length element represents a (length), it is
     * followed by a distance element, otherwise (literal) it is not.
     *
     * In DEFLATE, literal/length elements are encoded using
     * huffman literal/length tree, and distances are encoded
     * using huffman distance tree. we will use huffman to encode
     * data before it is read.
     *
     * this simple algorithm works as follows:
     * read a literal/length element, if it is literal
     * write the literal to the output stream, otherwise,
     * it is expected that next element is a distance,
     * read it! then process that pair of length/distance.
     */

    int32_t lit = readLiteral();

    if (lit < 256) {
        putByte(lit);
        return 0;
    } else if (lit == 256) {
        /* printf("EOB!\n"); */
        return 1;
    } else {
        int32_t len = lit-256;
        int32_t x = readDistance();
        putRange(x,len);
        return 0;
    }

}

/****************************************************************************/
/*                                 INFLATE                                  */
/****************************************************************************/

void decompress() {

    uint8_t BTYPE, BFINAL = 0;

    /* the compressed part is a series of compressed blocks,
     * loop on every block:
     */
    while(!BFINAL) {

        /* read block header: */
        BFINAL = getNextBit(); /* BFINAL */
        BTYPE = getNextN(2);

        if (BTYPE == 0) {
            /* no compression. */
            uint16_t LEN = 0, NLEN = 0;
            seekNextByte();
            LEN = getNextWord();
            NLEN = getNextWord();
            while(LEN--)
                putByte(getNextByte());
        } else {

            /* data is compressed. */

            if (BTYPE == 1) {
                /* fixed tables: */
                dynamic = 0;
            } else if (BTYPE == 2) {
                /* dynamic tables: */
                dynamic = 1;
                genDynamicTrees();
            } else {
                printf("Invalid BTYPE!\n");
                while(1);
            }

            /* read elements */
            while(!readNextElement());

        }

    }

}

/****************************************************************************/
/*                                  GZIP                                    */
/****************************************************************************/

struct {
    uint8_t ID1;
    uint8_t ID2;
    uint8_t CM;

    #define FTEXT        0x01
    #define FHCRC        0x02
    #define FEXTRA        0x04
    #define FNAME        0x08
    #define FCOMMENT    0x10
    uint8_t FLG;

    uint8_t MTIME[4];
    uint8_t XFL;
    uint8_t OS;
} gzHeader;

int32_t gzParseHeader() {
    seekNextByte();
    gzHeader.ID1      = getNextByte();
    gzHeader.ID2      = getNextByte();
    gzHeader.CM       = getNextByte();
    gzHeader.FLG      = getNextByte();
    gzHeader.MTIME[0] = getNextByte();
    gzHeader.MTIME[1] = getNextByte();
    gzHeader.MTIME[2] = getNextByte();
    gzHeader.MTIME[3] = getNextByte();
    gzHeader.XFL      = getNextByte();
    gzHeader.OS       = getNextByte();

    if (gzHeader.FLG & FEXTRA) {
        uint16_t XLEN = 0, i;
        XLEN |= (getNextByte() << 0);
        XLEN |= (getNextByte() << 8);
        while (i++ < XLEN)
            getNextByte();
    }

    if (gzHeader.FLG & FNAME) {
        char c;
        /* printf("Name: "); */
        do {
            c = getNextByte();
        /* printf("%c", c); */
        } while (c);
        /* printf("\n"); */
    }

    if (gzHeader.FLG & FCOMMENT) {
        uint8_t c;
        do c = getNextByte(); while (c);
    }

    if (gzHeader.FLG & FHCRC) {
        int16_t CRC16 = 0;
        CRC16 |= (getNextByte() << 0);
        CRC16 |= (getNextByte() << 8);
    }

    return 0;
}

void gzParseFooter() {

    uint32_t CRC32, ISIZE;

    seekNextByte();

    CRC32 = getNextLong();
    ISIZE = getNextLong();

    if (outp != ISIZE) {
        printf("\n");
        printf("Error happened during decompression!\n");
        while(1);
    } else {
        printf(" done\n");
        printf("Image Size: %dMB.\n", ISIZE/1024/1024);
        bootinfo->res[BI_RAMDISK].end =
             bootinfo->res[BI_RAMDISK].base + ISIZE;
    }

}

/****************************************************************************/
/*                                  main                                    */
/****************************************************************************/

void gunzip(char *file) {

    /* initialize streams: */
    streamInit(file);

    /* initialize huffman trees: */
    genFixedTrees();

    /* Read GNU ZIP Header: */
    gzParseHeader();

    /* Decompress */
    decompress();

    /* Parse Footer: */
    gzParseFooter();

}
