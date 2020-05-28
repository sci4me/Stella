#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

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
// TODO: Get rid of all these weird types!
typedef long int ptrdiff_t;
typedef long int intptr_t;
typedef long unsigned int uintptr_t;


#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <alloca.h>


#include "temporary_storage.cpp"


void __assert_fail(char const* msg, char const* file, s32 line);
#define assert(x) (static_cast<bool>(x) ? void(0) : __assert_fail(#x, __FILE__, __LINE__))


// NOTE TODO: Query this...
constexpr u64 PAGE_SIZE = 4096;

extern "C" {
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
	
	void* mlc_malloc(u64 n) {
		n += sizeof(u64); // NOTE: This is so we can store the size.
		
		u64 r = n % PAGE_SIZE;
		if(r > 0) {
			// NOTE: Align to page boundary.
			n += PAGE_SIZE - r;
		}

		void *ptr = sc_mmap(0, n, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		if(ptr == MAP_FAILED) return 0;

		*((u64*)ptr) = n;
		return ((u64*)ptr) + 1;
	}

	void* mlc_calloc(u64 n, u64 s) {
		// NOTE: We can do this since mlc_malloc is using mmap
		// w/ MAP_ANONYMOUS, which zero-initializes its results.
		return mlc_malloc(n * s);
	}

	void mlc_free(void *p) {
		if(p == 0) return;

		u64 *x = ((u64*)p) - 1;
		sc_munmap(x, *x);
	}

	void* mlc_realloc(void *p, u64 n) {
		void *p2 = mlc_malloc(n);
		if(!p2) return 0;
		
		if(!p) return p2;

		u64 old_n = *(((u64*) p) - 1) - sizeof(u64);
		if(n < old_n) n = old_n;
		mlc_memcpy(p2, p, n);

		mlc_free(p);
		return p2;
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

	void _mlc_qsort_swap(void *base, s64 size, s64 a, u64 b) {
		u8 *pa = ((u8*) base) + (a * size);
		u8 *pb = ((u8*) base) + (b * size);
		for(s64 i = 0; i < size; i++) {
			u8 t = *pa;
			*pa = *pb;
			*pb = t;
			pa++;
			pb++;
		}
	}

	s64 _mlc_qsort_partition(void *base, u64 size, s64 l, s64 h, s32 (*compar)(void const*, void const*)) {
		u8 *p_pivot = ((u8*) base) + (h * size);
		s64 i = l - 1;
		
		for(s64 j = l; j < h; j++) {
			u8 *pj = ((u8*) base) + (j * size);
			if(compar(pj, p_pivot) <= 0) {
				i++;
				_mlc_qsort_swap(base, size, i, j);
			}
		}

		_mlc_qsort_swap(base, size, i + 1, h);
		return i + 1;
	}

	void _mlc_qsort(void *base, u64 size, s64 p, s64 r, s32 (*compar)(void const*, void const*)) {
		if(p < r) {
			s64 q = _mlc_qsort_partition(base, size, p, r, compar);
			_mlc_qsort(base, size, p, q - 1, compar);
        	_mlc_qsort(base, size, q + 1, r, compar);
		}
	}

	void mlc_qsort(void* base, u64 num, u64 size, s32 (*compar)(void const*, void const*)) {
		_mlc_qsort(base, size, 0, (s64)num - 1, compar);
	}

	int mlc_sscanf(char const* s, char const* format, ...) {
		assert(0);
		// TODO
		return 0;
	}
}


char* tvsprintf(char const* fmt, va_list args) {
    va_list args2;
    va_copy(args2, args);
    s32 len = stbsp_vsnprintf(0, 0, fmt, args);
    va_end(args);

    char *buf = (char*) talloc(len);

    stbsp_vsprintf(buf, fmt, args2);
    va_end(args2);

    buf[len] = 0;

    return buf;
}

char* tsprintf(char const* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    return tvsprintf(fmt, args);
}

void tprintf(char const* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *buf = tvsprintf(fmt, args);
    sc_write(STDOUT, buf, mlc_strlen(buf) + 1);
}

void tfprintf(s32 fd, char const* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *buf = tvsprintf(fmt, args);
    sc_write(fd, buf, mlc_strlen(buf) + 1);
}


void __assert_fail(char const* msg, char const* file, s32 line) {
    tfprintf(STDERR, "Assertion failed: %s at %s, line %d\n", msg, file, line);

    s32 pid = sc_getpid();
    s32 tid = sc_gettid();
    sc_tgkill(pid, tid, 6); // SIGABRT
}