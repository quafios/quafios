/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios C Standard Library.                         | |
 *        | |  -> stdlib: malloc() and free() routines.            | |
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

/* the famous malloc() routine :) */

#include <stdlib.h>
#include <api/mman.h>

typedef struct chunk {
    struct chunk *next;
    struct chunk *prev;
    uint32_t used;
    uint32_t size;
} chunk_t;

static chunk_t *first = NULL;
static chunk_t *last  = NULL;
int chunk_count = 0;

void heap_init() {
    first = (chunk_t *) sbrk(sizeof(chunk_t));
    last  = first;
    first->next = NULL;
    first->prev = NULL;
    first->used = 1;
    first->size = 0;
    chunk_count = 1;
}

chunk_t *add_chunk(uint32_t size) {
    chunk_t *chunk = (chunk_t *) sbrk(sizeof(chunk_t)+size);
    chunk->next = NULL;
    chunk->prev = last;
    last->next = chunk;
    last = chunk;
    chunk->used = 0;
    chunk->size = size;
    chunk_count++;
    return chunk;
}

chunk_t *find_chunk(uint32_t size) {
    chunk_t *cur = first;
    while (cur) {
        if (!cur->used && cur->size >= size)
            break;
        cur = cur->next;
    }
    return cur;
}

void split(chunk_t *chunk, uint32_t newsize) {
    chunk_t *newchunk = (chunk_t *)(((uint32_t)chunk)+sizeof(chunk_t)+newsize);
    newchunk->prev = chunk;
    newchunk->next = chunk->next;
    newchunk->size = chunk->size-newsize-sizeof(chunk_t);
    newchunk->used = 0;
    chunk->next = newchunk;
    chunk->size = newsize;
    if (last == chunk)
        last = newchunk;
    chunk_count++;
}

chunk_t *merge(chunk_t *chunk) {
    chunk->size += chunk->next->size + sizeof(chunk_t);
    chunk->next = chunk->next->next;
    if (chunk->next) {
        chunk->next->prev = chunk;
    } else {
        last = chunk;
    }
    chunk_count--;
    return chunk;
}

void *malloc(size_t size) {
    chunk_t *chunk = find_chunk(size);
    if (!chunk) {
        /* add chunk */
        chunk = add_chunk(size);
    } else {
        /* split? */
        if (chunk->size > size+sizeof(chunk_t)) {
            split(chunk, size);
        }
    }
    chunk->used = 1;
    return (void *)(((uint32_t) chunk) + sizeof(chunk_t));
}

void free(void *ptr) {
    chunk_t *chunk = (chunk_t *)(((uint32_t) ptr) - sizeof(chunk_t));
    chunk->used = 0;
    if (chunk == last) {
        /* remove chunks */
        while (!last->used) {
            last = last->prev;
            last->next = NULL;
            chunk_count--;
            brk((void *)(((uint32_t) last) + sizeof(chunk_t) + last->size));
        }
    } else {
        /* merge? */
        if (!chunk->prev->used)
            chunk = merge(chunk->prev);
        if (!chunk->next->used)
            chunk = merge(chunk);
    }
}
