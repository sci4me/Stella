/*

sci4me's personal `stb.h`

No warranty is offered or implied; use this code at your own risk.

This code is intended to be compiled _only_ for 64-bit architectures.

See the end of this file for license info.

*/

/*

TODO:
 - More #defines
    For example, the ability to enable/disable parts of the library
 - Allow Arenas to have different block sizes?

*/

#ifndef SCI_H
#define SCI_H

#include <stdlib.h>
#include <assert.h>

//
// Static Assertion
//

#define sci_static_assert3(cond, msg) typedef char static_assertion_##msg[(!!(cond))*2-1]
#define sci_static_assert2(cond, line) sci_static_assert3(cond, static_assertion_at_line_##line)
#define sci_static_assert1(cond, line) sci_static_assert2(cond, line)
#define sci_static_assert(cond) sci_static_assert1(cond, __LINE__)

//
// Basic Data Types
//

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;
typedef float f32;
typedef double f64;

#ifndef __cplusplus
typedef u8 bool;
#define true 1
#define false 0
#endif

sci_static_assert(sizeof(u8) == 1);
sci_static_assert(sizeof(s8) == 1);
sci_static_assert(sizeof(u16) == 2);
sci_static_assert(sizeof(s16) == 2);
sci_static_assert(sizeof(u32) == 4);
sci_static_assert(sizeof(s32) == 4);
sci_static_assert(sizeof(u64) == 8);
sci_static_assert(sizeof(s64) == 8);
sci_static_assert(sizeof(f32) == 4);
sci_static_assert(sizeof(f64) == 8);

//
// Numeric Helper Macros
//

#define KILOBYTE(x) (x*1024)
#define MEGABYTE(x) (KILOBYTE(x)*1024)
#define GIGABYTE(x) (MEGABYTE(x)*1024)
#define TERABYTE(x) (GIGABYTE(x)*1024)

//
// Simple Memory Arena
//
// This memory arena allocates fixed-sized blocks of memory
// that it allocates memory for the user out of. It does not
// remove unused blocks automatically.
// 
// All memory blocks are allocated using `calloc`. 
// Also note that a single allocation from an Arena 
// mustn't be larger than the Arena's block size. 
// By default this is 64k. In debug mode, an allocation
// larger than the block size will fail an assertion.
// In a relase build, it will simply return null.
//

#ifndef SCI_H_ARENA_BLOCK_SIZE
#define SCI_H_ARENA_BLOCK_SIZE KILOBYTE(64)
#endif

#ifndef SCI_H_ARENA_ALIGNMENT
#define SCI_H_ARENA_ALIGNMENT 8
#endif

typedef struct Arena_Block {
    struct Arena_Block *prev;
    u8 *base;
    u64 used;
} Arena_Block;

typedef struct Arena {
    Arena_Block *head;
} Arena;

void arena_init(Arena *a);
void* arena_alloc(Arena *a, u64 n);
void* arena_calloc(Arena *a, u64 n);
void arena_free(Arena *a);


//
// Temporary Storage
//
// Temporary storage is a simple fixed-size buffer
// that can be used to store short-term data. The user
// is expected to call `treset();` to free up the allocated
// temporary data at known, controlled times within their
// code, such that they do not attempt to use any temporary
// allocation after the reset, and such that the buffer is not
// overrun.
//
// If an allocation fails because there is not enough space
// available in temporary storage, `calloc` is used as a fallback
// and an error is logged to `stderr`.
// Optionally, the user may define `SCI_H_TEMP_STORAGE_ASSERT_NO_OVERRUN`
// to enable a debug-mode assertion that this case does not occur.
//
// Temporary allocations are also guaranteed to be 0-initialized.
//

#ifndef SCI_H_TEMP_STORAGE_SIZE
#define SCI_H_TEMP_STORAGE_SIZE KILOBYTE(64)
#endif

void tinit(void);
void tfree(void);
void treset(void);
void* talloc(u64 n);

#endif

#ifdef SCI_H_IMPL

//
// Simple Memory Arena
//

void sci__arena_alloc_new_block(Arena *a) {
    Arena_Block *blk = (Arena_Block*) calloc(SCI_H_ARENA_BLOCK_SIZE, sizeof(u8));
    blk->prev = a->head;
    blk->base = ((u8*) blk) + sizeof(Arena_Block);
    blk->used = sizeof(Arena_Block);
    a->head = blk;
}

void arena_init(Arena *a) {
    a->head = 0;
    sci__arena_alloc_new_block(a);
}

void* arena_alloc(Arena *a, u64 x) {
    assert(x <= SCI_H_ARENA_BLOCK_SIZE);
    if(x > SCI_H_ARENA_BLOCK_SIZE) return 0;
    
    u64 n = (x + (SCI_H_ARENA_ALIGNMENT-1)) & ~(SCI_H_ARENA_ALIGNMENT-1);

    u64 free = SCI_H_ARENA_BLOCK_SIZE - a->head->used;
    if(n > free) {
        sci__arena_alloc_new_block(a);
    }

    void *result = a->head->base + a->head->used;
    a->head->used += n;
    return result;
}

void arena_free(Arena *a) {
    Arena_Block *curr = a->head;
    while(curr) {
        Arena_Block *next = curr->prev;
        free(curr);
        curr = next;
    }
}


//
// Temporary Storage
//

static u8 *sci__temporary_storage_buffer = 0;
static u64 sci__temporary_storage_used = 0;

void tinit(void) {
    assert(!sci__temporary_storage_buffer);
    sci__temporary_storage_buffer = (u8*) malloc(SCI_H_TEMP_STORAGE_SIZE);
}

void tfree(void) {
    assert(sci__temporary_storage_buffer);
    free(sci__temporary_storage_buffer);
}

void treset(void) {
    sci__temporary_storage_used = 0;
}

void* talloc(u64 x) {
    u64 n = (x + (SCI_H_ARENA_ALIGNMENT-1)) & ~(SCI_H_ARENA_ALIGNMENT-1);

#ifdef SCI_H_TEMP_STORAGE_ASSERT_NO_OVERRUN
    assert(n <= (SCI_H_TEMP_STORAGE_SIZE - sci__temporary_storage_used));
#else
    return calloc(n, sizeof(u8));
#endif

    u8 *result = sci__temporary_storage_buffer + sci__temporary_storage_used;
    sci__temporary_storage_used += n;
    return result;
}

#endif

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2020 Scitoshi Nakayobro
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/