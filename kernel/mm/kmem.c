/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> memman: kernel memory.                           | |
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
#include <sys/error.h>
#include <sys/mm.h>
#include <sys/semaphore.h>
#include <arch/page.h>
#include <tty/vtty.h>
#include <sys/printk.h>
#include <i386/stack.h>

#define L       5   /* 2^L is the smallest size block (32 bytes) */
#define U       30  /* 2^U is the upper size block (1GB)         */

/* linked lists */
typedef struct freenode {
    struct freenode *next;
    struct freenode *prev;
} __attribute__((packed)) freenode_t;

struct list {
    struct freenode *first;
    struct freenode *last;
    uint32_t count;
} __attribute__((packed)) freelist[33];

/* bitmaps */
#define BITMAP_SIZE     ((1<<(U-L+1))/8)

typedef struct submap {
    uint32_t start; /* bit offset in bitmap array */
} submap_t;

static submap_t submaps[33] = {0};
static uint8_t *bitmap = NULL;

/* binary operations */
#define pow2(n)         (((uint32_t) 1<<n))
#define nextpow2(n)     (__extension__({            \
                             uint32_t num = n;  \
                            --num;                  \
                            num |= num >> 1;        \
                            num |= num >> 2;        \
                            num |= num >> 4;        \
                            num |= num >> 8;        \
                            num |= num >> 16;       \
                            ++num;                  \
                            num;                    \
                        }))

/* Reference for LogDeBruijn:
 * http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn
 */

static const int MultiplyDeBruijnBitPosition2[32] = {
  0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
  31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

#define log2(n) (MultiplyDeBruijnBitPosition2[\
                    (uint32_t)(n * 0x077CB531U) >> 27])

/* runtime data */
int32_t kmem_initialized = 0;
uint32_t kalloc_size = 0;

/* ======================================================================== */
/*                           Bitmap Operations                              */
/* ======================================================================== */

void set_bit(uint32_t i, uint32_t bitno) {
    uint32_t base = (submaps[i].start+bitno)/8;
    uint32_t off  = (submaps[i].start+bitno)%8;
    if (i < L || i > U) {
        printk("kmem.c: set_bit() catched a bug! i: %d\n", i);
        vtty_ioctl(system_console, TTY_ENABLE, NULL);
        while(1);
    }
    if (bitmap) {
        bitmap[base] |= 1<<off;
    }
}

uint32_t get_bit(uint32_t i, uint32_t bitno) {
    uint32_t base = (submaps[i].start+bitno)/8;
    uint32_t off  = (submaps[i].start+bitno)%8;
    if (i < L || i > U) {
        printk("kmem.c: get_bit() catched a bug! i: %d\n", i);
        vtty_ioctl(system_console, TTY_ENABLE, NULL);
        while(1);
    }
    if (!arch_vmpage_getAddr(NULL, &bitmap[base])) {
        printk("kmem.c: get_bit() catched a bug (2)!\n");
        vtty_ioctl(system_console, TTY_ENABLE, NULL);
        while(1);
    }
    if (bitmap) {
        return (bitmap[base]>>off)&1;
    } else {
        return 1; /* free by default */
    }
}

void clear_bit(uint32_t i, uint32_t bitno) {
    uint32_t base = (submaps[i].start+bitno)/8;
    uint32_t off  = (submaps[i].start+bitno)%8;
    if (i < L || i > U) {
        printk("kmem.c: clear_bit() catched a bug!\n");
        vtty_ioctl(system_console, TTY_ENABLE, NULL);
        while(1);
    }
    if (bitmap) {
        bitmap[base] &= ~(1<<off);
    }
}

int32_t get_level(uint32_t off) {
    /* takes offset, returns its i */
    int32_t i, ptr = 0;
    for (i = U; i >= L; i--) {
        /* the purpose is to traverse the bitmap in order to find
         * the size of a reservation that starts with address "off".
         *
         * now ptr refers to a candidate node at level i,
         * off might be referring to this node actually
         * or it might be referring to one of its children
         *
         * if the node is free, this is an invalid case.
         * if the node is allocated:
         *   --> if both of its children are free, or we have reached
         *       the L level, then ptr must be the node we are
         *       looking for (allocated node by kmalloc())
         *   --> if at most one of its children if free, then we must go down
         *       one level again. we should choose the node that ptr
         *       lies into.
         */
        if (get_bit(i, ptr>>i)) {
            /* the node is free, this is an invalid case */
            return -1;
        }
        /* node is allocated */
        /*printk("i: %d, ptr: %x, children: %d %d %d %d %x\n", i, ptr,
            (ptr>>(i-1)),
            (ptr>>(i-1))+1,
            i == L ? 1:get_bit(i-1,(ptr>>(i-1))),
            i == L ? 1:get_bit(i-1,(ptr>>(i-1))+1),
            submaps[i-1].start
        );*/
        if (i == L||(get_bit(i-1,(ptr>>(i-1)))&&get_bit(i-1,(ptr>>(i-1))+1))) {
            /* node is allocated by kmalloc()... */
            if (ptr == off)
                return i;
            else
                return -1; /* this is impossible */
        }
        /* find the child which "off" could lie in */
        ptr = (off >= ptr+(1<<(i-1))) ? (ptr+(1<<(i-1))):ptr;
        /*printk("new ptr: %x\n", ptr);*/
    }
}

/* ======================================================================== */
/*                           Linked Allocation                              */
/* ======================================================================== */

void linked_init(int32_t i) {
    freelist[i].first = NULL;
    freelist[i].last = NULL;
    freelist[i].count = 0;
}

void linked_add(int32_t i, freenode_t *ptr) {
    if (freelist[i].first) {
        freelist[i].first->prev = ptr;
    }
    ptr->next = freelist[i].first;
    ptr->prev = NULL;
    freelist[i].first = ptr;
    if (!freelist[i].last)
        freelist[i].last = ptr;
    freelist[i].count++;
}

void linked_remove(int32_t i, freenode_t *ptr) {
    if (ptr->prev) {
        ptr->prev->next = ptr->next;
    } else {
        freelist[i].first = ptr->next;
    }
    if (ptr->next) {
        ptr->next->prev = ptr->prev;
    } else {
        freelist[i].last = ptr->prev;
    }
    freelist[i].count--;
}

freenode_t *get_free_node(int32_t i) {
    freenode_t *ptr;
    /* integrity check */
    if (i < L || i > U) {
        printk("kmem.c: get_free_node() catched a bug!\n");
        vtty_ioctl(system_console, TTY_ENABLE, NULL);
        while(1);
    }
    /* lookup the linked list size */
    if (!freelist[i].count)
        return NULL;
    /* remove the head of the linked list */
    linked_remove(i, ptr = freelist[i].first);
    /* clear the bit in the bitmap corresponding to the node */
    clear_bit(i,(((uint32_t) ptr)-KERNEL_MEMORY_BASE)>>i); /* set as used */
    /* set both children as free */
    if (i > L) {
        set_bit(i-1,((((uint32_t) ptr)-KERNEL_MEMORY_BASE)>>(i-1))+0);
        set_bit(i-1,((((uint32_t) ptr)-KERNEL_MEMORY_BASE)>>(i-1))+1);
    }
    /* deallocate page at ptr if node size is >= PAGE_SIZE */
    if (i > 11)
        arch_vmpage_unmap(NULL, ptr);
    /* done */
    return ptr;
}

void put_free_node(int32_t i, freenode_t *ptr) {
    /* integrity check */
    if (i < L || i > U) {
        printk("kmem.c: put_free_node() catched a bug!\n");
        vtty_ioctl(system_console, TTY_ENABLE, NULL);
        while(1);
    }
    /* allocate the page at which ptr begins (so we can access it) */
    arch_vmpage_map(NULL, ptr, 0);
    /* add to linked list */
    linked_add(i, ptr);
    /* set the bit in the bitmap corresponding to the node */
    set_bit(i,(((uint32_t) ptr)-KERNEL_MEMORY_BASE)>>i); /* set as free */
}

int32_t get_node_level(freenode_t *ptr) {
    return get_level(((uint32_t) ptr)-KERNEL_MEMORY_BASE);
}

/* ======================================================================== */
/*                          Allocation Functions                            */
/* ======================================================================== */

freenode_t *get_hole(uint32_t i) {
    /* buddy system reservation */
    freenode_t *ptr;
    uint32_t base, j;
    /* make sure size is not bigger than the maximum */
    if (i > U) {
        /* size can't be more than 2^U */
        Regs regs = {0};
        panic(&regs, "Kernel memory not enough!");
    }
    /* make sure size is not less than the minimum */
    if (i < L) {
        /* size can't be less than 2^L */
        i = L;
    }
    /* get a free hole of size i from the linked lists & bitmaps */
    ptr = get_free_node(i);
    /* is ptr valid? */
    if (!ptr) {
        /* ptr is not valid, we must get a bigger chunk and split it */
        ptr = get_hole(i+1);
        /* split returned chunk into two (2^i) chunks */
        put_free_node(i, (freenode_t *) (((uint32_t) ptr) + pow2(i)));
        /* set buddy as free */
        set_bit(i,((((uint32_t) ptr)+pow2(i))-KERNEL_MEMORY_BASE)>>i);
        /* set ptr as nonfree node */
        clear_bit(i,(((uint32_t) ptr)-KERNEL_MEMORY_BASE)>>i);
        /* set children of ptr as free */
        if (i > L) {
            set_bit(i-1,((((uint32_t) ptr)-KERNEL_MEMORY_BASE)>>(i-1))+0);
            set_bit(i-1,((((uint32_t) ptr)-KERNEL_MEMORY_BASE)>>(i-1))+1);
        }
    }
    /* allocate physical pages: */
    base = (uint32_t) ptr;
    for (j = base & 0xFFFFF000; j < base+(1<<i); j += PAGE_SIZE)
        arch_vmpage_map(NULL, j, 0);
    /* return ptr */
    return ptr;
}

void put_hole(uint32_t i, freenode_t *ptr) {
    /* buddy system liberation */
    uint32_t size = pow2(i), nextsize, nextbits, j;
    uint32_t cur = (uint32_t) ptr, buddy, next;
    /* find buddy */
    buddy = cur ^ size;
    /* find parent */
    nextsize = size * 2;
    nextbits = i + 1;
    next = cur & buddy;
    /* deallocate the pages of the hole */
    if (i > 11) {
        /* hole is >= one page */
        for (j = 0; j < size; j+= PAGE_SIZE)
            arch_vmpage_unmap(NULL, cur+j);
    }
    /* is buddy free? */
    if (!get_bit(i, (buddy-KERNEL_MEMORY_BASE)>>i)) {
        /* buddy is not free, just add this node as free node */
        put_free_node(i, ptr);
    } else {
        /* buddy is free, join */
        linked_remove(i, (freenode_t *) buddy);
        put_hole(nextbits, (freenode_t *) next);
    }
}

/* ======================================================================== */
/*                          Allocation Functions                            */
/* ======================================================================== */

void *kmalloc(uint32_t size) {
    /* local vars */
    freenode_t *ptr;

    /* size must be a power of 2 */
    size = nextpow2(size);

    /* size must be 2^L at minimum */
    if (size <= (1<<L))
        size = 1<<L;

    /* Get free hole: */
    ptr = get_hole(log2(size));

    /* increase counter */
    kalloc_size += size;

    /* return pointer to allocated area */
    return ptr;
}

void kfree(void *ptr) {
    /* local vars */
    int32_t i;

    /* get size */
    i = get_node_level(ptr);

    /* invalid ptr? */
    if (i < 0) {
        printk("kfree(): invalid ptr %x, caller %x.\n",ptr,((int *)&ptr)[-1]);
        vtty_ioctl(system_console, TTY_ENABLE, NULL);
        while(1);
    }

    /* release the node */
    put_hole(i, ptr);

    /* decrease counter */
    kalloc_size -= 1<<i;
}

/* ======================================================================== */
/*                              Initialization                              */
/* ======================================================================== */

void kmem_init() {

    int32_t i, j;
    char *buf;

    /* initialize free lists: */
    for (i = 0; i < 33; i++)
        linked_init(i);

    /* create root node */
    arch_vmpage_map(NULL, KERNEL_MEMORY_BASE, 0);
    linked_add(U, (freenode_t *)((uint32_t) KERNEL_MEMORY_BASE));

    /* create bitmap structure */
    bitmap = (uint8_t *) kmalloc(BITMAP_SIZE);

    /* set submap structures */
    for (i = U, j = 0; i >= L; i--) {
        submaps[i].start = j;
        j+=1<<(U-i);
    }

    /* initialize the bitmap */
    i = log2(nextpow2(BITMAP_SIZE))-1;
    set_bit(i, 0);
    set_bit(i, 1);
    for (i++; i < U; i++) {
        clear_bit(i, 0); /* level[i].chunk[0] is allocated */
        set_bit(i, 1); /* level[i].chunk[1] is free */
    }
    clear_bit(i, 0); /* root is allocated */

    /* kmem is initialized */
    kmem_initialized = 1;

}
