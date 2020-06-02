#include "mylibc.hpp"

// NOTE TODO: Query this...
constexpr u64 PAGE_SIZE = 4096;

extern "C" {
	void* mlc_memset(void *p, s32 v, u64 n) {
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

	s32 mlc_memcmp(void const* a, void const* b, u64 n) {
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

	void const* mlc_memchr(void const* p, s32 v, u64 n) {
		u8 const* d = (u8 const*) p;
		for(u64 i = 0; i < n; i++) {
			if(*d == v) return d;
			d++;
		}
		return 0;
	}

	s32 mlc_strcmp(char const* a, char const* b) {
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

	char const* mlc_strchr(char const* s, s32 c) {
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

	s32 mlc_strncmp(char const* a, char const* b, u64 n) {
		for(u64 i = 0; i < n; i++) {
			u8 x = *a++;
			u8 y = *b++;
			if(x == 0 || y == 0) return 0;
			if(x < y) return -1;
			if(x > y) return 1;
		}
		return 0;
	}

	s64 mlc_strtol(const char* str, char** endptr, s32 base) {
		assert(0);
		// TODO
		return 0;
	}

	f32 mlc_atof(char const* s) {
		assert(0);
		// TODO
		return 0;
	}

	s32 mlc_toupper(s32 x) {
		assert(0);
		// TODO
		return x;
	}

	void _mlc_qsort_swap(u8 *a, u8 *b, u64 size) {
		for(u64 i = 0; i < size; i++) {
			u8 t = *a;
			*a = *b;
			*b = t;
			a++;
			b++;
		}
	}

	s64 _mlc_qsort_partition(void *base, u64 size, s64 l, s64 h, s32 (*compar)(void const*, void const*)) {
		u8 *p_pivot = ((u8*) base) + (h * size);
		s64 i = l - 1;
		
		for(s64 j = l; j < h; j++) {
			u8 *pj = ((u8*) base) + (j * size);
			if(compar(pj, p_pivot) <= 0) {
				i++;

				u8 *pi = ((u8*) base) + (i * size);
				_mlc_qsort_swap(pi, pj, size);
			}
		}

		u8 *pi = ((u8*) base) + ((i + 1) * size);
		u8 *ph = ((u8*) base) + (h * size);
		_mlc_qsort_swap(pi, ph, size);

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
		if(num <= 1) return;
		// NOTE TODO BUG: 2000 bucks says there's a bug in this code path somewhere
		_mlc_qsort(base, size, 0, (s64)num - 1, compar);
	}

	s32 mlc_sscanf(char const* s, char const* format, ...) {
		assert(0);
		// TODO
		return 0;
	}
}