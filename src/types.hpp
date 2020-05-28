#ifndef TYPES_H
#define TYPES_H

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

// TODO: some (much) of this needs to be temporary.
// TODO: Get rid of all these weird types!
typedef long int ptrdiff_t;
typedef long int intptr_t;
typedef long unsigned int uintptr_t;

#ifndef NULL
#define NULL 0
#endif

#endif