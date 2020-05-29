//-----------------------------------------------------------------------------
// COMPILE-TIME OPTIONS FOR DEAR IMGUI
// Runtime options (clipboard callbacks, enabling various features, etc.) can generally be set via the ImGuiIO structure.
// You can use ImGui::SetAllocatorFunctions() before calling ImGui::CreateContext() to rewire memory allocation functions.
//-----------------------------------------------------------------------------
// A) You may edit imconfig.h (and not overwrite it when updating Dear ImGui, or maintain a patch/branch with your modifications to imconfig.h)
// B) or add configuration directives in your own file and compile with #define IMGUI_USER_CONFIG "myfilename.h"
// If you do so you need to make sure that configuration settings are defined consistently _everywhere_ Dear ImGui is used, which include
// the imgui*.cpp files but also _any_ of your code that uses Dear ImGui. This is because some compile-time options have an affect on data structures.
// Defining those options in imconfig.h will ensure every compilation unit gets to see the same data structure layouts.
// Call IMGUI_CHECKVERSION() from your .cpp files to verify that the data structures your files are using are matching the ones imgui.cpp is using.
//-----------------------------------------------------------------------------

#pragma once


#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <alloca.h>

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

#ifndef NULL
#define NULL 0
#endif


// NOTE TODO(?): Compatibility bs...
typedef s64 ptrdiff_t;
typedef s64 intptr_t;
typedef u64 uintptr_t;


#define IMGUI_NO_LIBC


extern "C" {
	void* mlc_memset(void *p, s32 v, u64 n);
	void* mlc_memmove(void *dst, void const* src, u64 n);
	void* mlc_memcpy(void *dst, void const* src, u64 n);
	s32 mlc_memcmp(void const* a, void const* b, u64 n);
	u64 mlc_strlen(char const* s);
	char const* mlc_strchr(char const* s, s32 c);
	char* mlc_strcpy(char *dst, char const* src);
	s32 mlc_strcmp(char const* a, char const* b);
	f32 mlc_atof(char const* s);
	void mlc_qsort(void* base, u64 num, u64 size, s32 (*compar)(void const*, void const*));
	void* mlc_malloc(u64 n);
	void mlc_free(void *p);
	s32 mlc_toupper(s32 x);
	char* mlc_strncpy(char *dst, char const* src, u64 n);
	void const* mlc_memchr(void const* p, s32 v, u64 n);
	char const* mlc_strstr(char const* a, char const* b);
	s32 mlc_sscanf(char const* s, char const* format, ...);

	s32 stbsp_vsnprintf(char * buf, s32 count, char const* fmt, va_list va);

	f32 PT_fabsf(f32 x);
	f32 PT_sqrtf(f32 x);
	f32 PT_fmodf(f32 x, f32 y);
	f64 PT_cos(f64 x);
	f64 PT_sin(f64 x);
	f32 PT_acosf(f32 x);
	f32 PT_atan2f(f32 y, f32 x);
	f32 PT_floorf(f32 x);
	f32 PT_ceilf(f32 x);
	f32 PT_powf(f32 b, f32 e);
	f64 PT_pow(f64 b, f64 e);
}


// #define IM_ASSERT(x)                assert(x)
#define IM_ASSERT(x)
#define IM_MEMSET(d, x, n)          mlc_memset(d, x, n)
#define IM_MEMMOVE(d, s, n)         mlc_memmove(d, s, n)
#define IM_MEMCPY(d, s, n)          mlc_memcpy(d, s, n)
#define IM_MEMCMP(a, b, n)          mlc_memcmp(a, b, n)
#define IM_STRLEN(s)                mlc_strlen(s)
#define IM_STRCHR(s, c)             mlc_strchr(s, c)
#define IM_STRCPY(d, s)             mlc_strcpy(d, s)
#define IM_STRCMP(a, b)             mlc_strcmp(a, b)
#define ImFabs(x)                   PT_fabsf(x)
#define ImSqrt(x)                   PT_sqrtf(x)
#define ImFmod(a, b)                PT_fmodf(a, b)
#define ImCos(x)                    PT_cos(x)
#define ImSin(x)                    PT_sin(x)
#define ImAcos(x)                   PT_acosf(x)
#define ImAtan2(y, x)               PT_atan2f(y, x)
#define ImFloorStd(x)               PT_floorf(x)
#define ImCeil(x)                   PT_ceilf(x)
#define ImAtof(s)                   mlc_atof(s)
static inline float ImPow(float b, float e) { return PT_powf(b, e); }
static inline double ImPow(double b, double e) { return PT_pow(b, e); }
#define ImQsort(b, n, s, c)         mlc_qsort(b, n, s, c)
#define IM_MALLOC_FN(n)             mlc_malloc(n)
#define IM_FREE_FN(p)               mlc_free(p)
#define IM_TOUPPER(c)               mlc_toupper(c)
#define IM_STRNCPY(d, s, n)         mlc_strncpy(d, s, n)
#define IM_MEMCHR(p, v, n)          mlc_memchr(p, v, n)
#define IM_STRSTR(a, b)             mlc_strstr(a, b)
#define IM_SSCANF(s, f, ...)        mlc_sscanf(s, f, __VA_ARGS__)
#define IM_VSNPRINTF(fmt, ...)      stbsp_vsnprintf(fmt, __VA_ARGS__)