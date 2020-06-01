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


// NOTE: Yes, I do enjoy being this rude. Fite me.
// (Currently I only target 64-bit architectures. x64 specifically :P)
static_assert(sizeof(u8) == 1);
static_assert(sizeof(s8) == 1);
static_assert(sizeof(u16) == 2);
static_assert(sizeof(s16) == 2);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(s32) == 4);
static_assert(sizeof(u64) == 8);
static_assert(sizeof(s64) == 8);
static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);


#ifndef NULL
#define NULL 0
#endif


#define array_length(a) ((sizeof(a))/(sizeof(a[0])))
#define offsetof(type, member) ((u64)&(((type *)0)->member))


// TODO: Make this less terrible.
#define assert(x) if(!(x)){*(volatile int*)0=0;}


#endif