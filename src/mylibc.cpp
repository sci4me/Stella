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


// TODO: some (much) of this needs to be temporary.
#define NULL 0
typedef long unsigned int size_t;
typedef long int ptrdiff_t;
typedef long int intptr_t;
typedef long unsigned int uintptr_t;


#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <alloca.h>


extern "C" {
	void* mlc_malloc(u64 n) {
		// TODO
		return 0;
	}

	void* mlc_realloc(void *p, u64 n) {
		// TODO
		return 0;
	}

	void mlc_free(void *p) {
		// TODO
	}

	void* mlc_memset(void *p, int v, u64 n) {
		// TODO
		return p;
	}

	void* mlc_memcpy(void *dst, void const* src, u64 n) {
		// TODO
		return dst;
	}

	int mlc_memcmp(void const* a, void const* b, size_t n) {
		// TODO
		return 0;
	}

	void* mlc_memmove(void *dst, void const* src, size_t n) {
		// TODO
		return 0;
	}

	void const* mlc_memchr(void const* p, int v, size_t n) {
		// TODO
		return 0;
	}

	int mlc_strcmp(char const* a, char const* b) {
		// TODO
		return 0;
	}

	size_t mlc_strlen(char const* s) {
		// TODO
		return 0;
	}

	char const* mlc_strchr(char const* s, int c) {
		// TODO
		return 0;
	}

	char const* mlc_strstr(char const* a, char const* b) {
		// TODO
		return 0;
	}

	char* mlc_strcpy(char *dst, char const* src) {
		// TODO
		return 0;
	}

	char* mlc_strncpy(char *dst, char const* src, size_t n) {
		// TODO
		return 0;
	}

	int mlc_strncmp(char const* a, char const* b, u64 n) {
		// TODO
		return 0;
	}

	long int mlc_strtol(const char* str, char** endptr, int base) {
		// TODO
		return 0;
	}

	float mlc_atof(char const* s) {
		// TODO
		return 0;
	}

	int mlc_toupper(int x) {
		// TODO
		return x;
	}

	void mlc_qsort(void* base, size_t num, size_t size, int (*compar)(void const*, void const*)) {
		// TODO
	}

	int mlc_sscanf(char const* s, char const* format, ...) {
		// TODO
		return 0;
	}
}