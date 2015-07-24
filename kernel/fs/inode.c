/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 2.0.1.                               | |
 *        | |  -> Filesystem: Inode Cache.                         | |
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
#include <sys/mm.h>
#include <sys/fs.h>
#include <lib/linkedlist.h>

#define IHASH_COUNT     0x40000

typedef _linkedlist(inode_t) ill; /* inode linked list. */

ill *ihashtable[IHASH_COUNT]; /* hash table of inode. */

void icache_init() {
    /* initalize the hash table */
    int32_t i;
    for (i = 0; i < IHASH_COUNT; i++)
        ihashtable[i] = NULL;
}

int32_t hash(super_block_t *sb, ino_t ino) {
    /* the hashing function */
    uint32_t isb = (uint32_t) sb;
    int32_t  ret = 0;
    ret += (ino>> 0) & 0xFFFF;
    ret += (ino>>16) & 0xFFFF;
    ret += (isb>> 0) & 0xFFFF;
    ret += (isb>>16) & 0xFFFF;
    return ret;
}

inode_t *iget(super_block_t *sb, ino_t ino) {

    /* get the inode_t structure. */
    int32_t h;
    inode_t *p;

    /* get the hashing value */
    h = hash(sb, ino);

    if (ihashtable[h] == NULL) {
        /* allocate the linkedlist. */
        if (!(ihashtable[h] = kmalloc(sizeof(ill))))
            return NULL;

        /* initialize the linked list. */
        linkedlist_init(ihashtable[h]);
    }

    /* loop on the linked list until we find the inode: */
    p = ihashtable[h]->first;
    while(p && (p->sb != sb || p->ino != ino))
        p = p->next;

    /* the desired inode doesn't exist? allocate it! */
    if (!p) {

        /* allocate: */
        if (!(p = kmalloc(sizeof(inode_t))))
            return NULL;

        /* add to the linked list */
        linkedlist_addlast(ihashtable[h], p);

        /* initialize: */
        p->sb  = sb;
        p->ino = ino;
        p->icount = 0;
        p->dev = NULL;
        p->submount = NULL;

        /* initialized sma */
        linkedlist_init(&(p->sma));

        /* read from disk: */
        sb->fsdriver->read_inode(p);

        /* translate devid: */
        if ((p->mode & FT_MASK) == FT_SPECIAL)
            p->dev = (device_t *) devid_to_dev(p->devid);
    }

    /* now p refers to the desired inode structure, update the counters: */
    p->icount++;
    sb->icount++;

    /* done: */
    return p;

}

void iput(inode_t *p) {

    int32_t h;

    /* decrease the counters: */
    p->icount--;
    p->sb->icount--;

    /* do file system specific stuff: */
    p->sb->fsdriver->put_inode(p);

    /* p is still used? */
    if (p->icount)
        return;

    /* get the hash value: */
    h = hash(p->sb, p->ino); /* get the hashing value */

    /* remove from linked list: */
    linkedlist_aremove(ihashtable[h], p);

    /* linked list is free? */
    if (!(ihashtable[h]->count)) {
        kfree(ihashtable[h]);
        ihashtable[h] = NULL;
    }

    /* unallocate the inode structure */
    kfree(p);

}
