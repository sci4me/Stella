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

// TODO: Conditional includes for Winderps and Linux.
#include "linux_syscall.cpp"

#ifndef NULL
#define NULL 0
#endif

// TODO: some (much) of this needs to be temporary.
typedef long int ptrdiff_t;
typedef long int intptr_t;
typedef long unsigned int uintptr_t;


#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <alloca.h>

void __assert_fail(char const* msg, char const* file, s32 line);
#define assert(x) (static_cast<bool>(x) ? void(0) : __assert_fail(#x, __FILE__, __LINE__))

// TODO: Get rid of all these weird types!

extern "C" {
	void* mlc_malloc(u64 n) {
		assert(0);
		// TODO
		return 0;
	}

	void* mlc_calloc(u64 n, u64 s) {
		assert(0);
		// TODO
		return 0;
	}

	void* mlc_realloc(void *p, u64 n) {
		assert(0);
		// TODO
		return 0;
	}

	void mlc_free(void *p) {
		assert(0);
		// TODO
	}

	void* mlc_memset(void *p, int v, u64 n) {
		// TODO: SIMD version of this!!!!!
		u8 *real_p = (u8*) p;
		for(u64 i = 0; i < n; i++) real_p[i] = v;
		return p;
	}

	void* mlc_memcpy(void *dst, void const* src, u64 n) {
		// TODO: SIMD version of this!!!!!
		u8 *d = (u8*) dst;
		u8 const* s = (u8 const*) src;
		for(u64 i = 0; i < n; i++) d[i] = s[i];
		return dst;
	}

	int mlc_memcmp(void const* a, void const* b, u64 n) {
		u8 const* l = (u8 const*) a;
		u8 const* r = (u8 const*) b;
		for(u64 i = 0; i < n; i++) {
			u8 x = *l++;
			u8 y = *r++;
			if(x < y) return -1;
			if(x > y) return 1;
		}
		return 0;
	}

	void* mlc_memmove(void *dst, void const* src, u64 n) {
		if(dst < src) {
			mlc_memcpy(dst, src, n);
		} else {
			// TODO: SIMD version of this!!!!!
			u8 *d = (u8*) dst;
			u8 const* s = (u8 const*) src;
			for(u64 i = n; i > 0; i--) d[i - 1] = s[i - 1]; 
		}
		return dst;
	}

	void const* mlc_memchr(void const* p, int v, u64 n) {
		u8 const* d = (u8 const*) p;
		for(u64 i = 0; i < n; i++) {
			if(*d == v) return d;
			d++;
		}
		return 0;
	}

	int mlc_strcmp(char const* a, char const* b) {
		for(; *a == *b && *a; a++, b++);
  		return *(u8 const*)a - *(u8 const*)b;
	}

	u64 mlc_strlen(char const* s) {
		// TODO: Better version of this?
		// We could use SIMD, for one thing.
		u64 xd = 0;
		for(char const* c = s; *c; c++) xd++;
		return xd;
	}

	char const* mlc_strchr(char const* s, int c) {
		do {
			if(*s == c) return s;
		} while(*s++);
		return 0;
	}

	char const* mlc_strstr(char const* a, char const* b) {
		// NOTE TODO: We just did the naive implementation!
		// It's really bad! Like, O(mn) time. SIMD-ize it! :P
		u64 n = mlc_strlen(b);
		while(*a) {
			if(mlc_memcmp(a, b, n) == 0) return a;
			a++;
		}
		return 0;
	}

	char* mlc_strcpy(char *dst, char const* src) {
		char *d = dst;
		for(char const* c = src; *c; c++) *d++ = *c;
		return dst;
	}

	char* mlc_strncpy(char *dst, char const* src, u64 n) {
		// TODO: More efficient version of this!!!
		// No need to call strlen!!! Should probably(?)
		// be able to remove that branch too.
		auto len = mlc_strlen(src);
		for(u64 i = 0; i < n; i++) {
			if(i < len) {
				dst[i] = src[i];
			} else {
				dst[i] = 0;
			}
		}
		// NODE TODO: If we need to zero the last N bytes, 
		// why not just use memset to do this?
		return dst;
	}

	int mlc_strncmp(char const* a, char const* b, u64 n) {
		for(u64 i = 0; i < n; i++) {
			u8 x = *a++;
			u8 y = *b++;
			if(x == 0 || y == 0) return 0;
			if(x < y) return -1;
			if(x > y) return 1;
		}
		return 0;
	}

	long int mlc_strtol(const char* str, char** endptr, int base) {
		assert(0);
		// TODO
		return 0;
	}

	float mlc_atof(char const* s) {
		assert(0);
		// TODO
		return 0;
	}

	int mlc_toupper(int x) {
		assert(0);
		// TODO
		return x;
	}

	void mlc_qsort(void* base, u64 num, u64 size, int (*compar)(void const*, void const*)) {
		assert(0);
		// TODO
	}

	int mlc_sscanf(char const* s, char const* format, ...) {
		assert(0);
		// TODO
		return 0;
	}
}