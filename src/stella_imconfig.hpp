#pragma once

#include "common.hpp"
#include "mylibc.hpp"


#include "stb_sprintf.h"


#include <stdint.h>

#ifdef STELLA_OS_WINDOWS
#include <malloc.h>
#else
#include <alloca.h>
#endif


extern "C" {
	void* stella_im_malloc(u64);
	void stella_im_free(void*);

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


#define IMGUI_NO_LIBC
#define IM_ALLOCA					alloca
#define IM_ASSERT(x)                assert(x)
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
#define IM_MALLOC_FN(n)             stella_im_malloc(n)
#define IM_FREE_FN(p)               stella_im_free(p)
#define IM_TOUPPER(c)               mlc_toupper(c)
#define IM_STRNCPY(d, s, n)         mlc_strncpy(d, s, n)
#define IM_MEMCHR(p, v, n)          mlc_memchr(p, v, n)
#define IM_STRSTR(a, b)             mlc_strstr(a, b)
#define IM_SSCANF(s, f, ...)        mlc_sscanf(s, f, __VA_ARGS__)
#define IM_VSNPRINTF(fmt, ...)      stbsp_vsnprintf(fmt, __VA_ARGS__)