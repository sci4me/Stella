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

#include <limits.h>

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

	int mlc_abs(int x) {
		// TODO
		return x;
	}

	int mlc_strcmp(char const* a, char const* b) {
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

	float mlc_ldexpf(float f, int x) {
		// TODO
		return 0;
	}

	float mlc_powf(float base, float exp) {
		// TODO
		return 0;
	}
}