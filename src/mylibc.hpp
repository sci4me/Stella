#ifndef MYLIBC_H
#define MYLIBC_H

#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <alloca.h>

#include "stb_sprintf.h"

#include "types.hpp"
#include "temporary_storage.hpp"


#define STDIN 				0
#define STDOUT				1
#define STDERR				2


extern "C" {
	void* mlc_memset(void *p, s32 v, u64 n);
	void* mlc_memcpy(void *dst, void const* src, u64 n);
	void* mlc_malloc(u64 n);
	void* mlc_calloc(u64 n, u64 s);
	void mlc_free(void *p);
	void* mlc_realloc(void *p, u64 n);
	s32 mlc_memcmp(void const* a, void const* b, u64 n);
	void* mlc_memmove(void *dst, void const* src, u64 n);
	void const* mlc_memchr(void const* p, s32 v, u64 n);
	int mlc_strcmp(char const* a, char const* b);
	u64 mlc_strlen(char const* s);
	char const* mlc_strchr(char const* s, s32 c);
	char const* mlc_strstr(char const* a, char const* b);
	char* mlc_strcpy(char *dst, char const* src);
	char* mlc_strncpy(char *dst, char const* src, u64 n);
	s32 mlc_strncmp(char const* a, char const* b, u64 n);
	long int mlc_strtol(const char* str, char** endptr, s32 base); // TODO: long int?
	f32 mlc_atof(char const* s);
	s32 mlc_toupper(s32 x);
	void mlc_qsort(void* base, u64 num, u64 size, s32 (*compar)(void const*, void const*));
	s32 mlc_sscanf(char const* s, char const* format, ...);
	void mlc_exit(s32 code);
}


struct Entire_File {
	u8 *data;
	u64 len;

	void deinit() { if(data) mlc_free(data); }
};

Entire_File read_entire_file(char const* name);


char* tvsprintf(char const* fmt, va_list args);
char* tsprintf(char const* fmt, ...);
void tprintf(char const* fmt, ...);
void tfprintf(s32 fd, char const* fmt, ...);


void __assert_fail(char const* msg, char const* file, s32 line);
#define assert(x) (static_cast<bool>(x) ? void(0) : __assert_fail(#x, __FILE__, __LINE__))

#define array_length(a) ((sizeof(a))/(sizeof(a[0])))

#endif