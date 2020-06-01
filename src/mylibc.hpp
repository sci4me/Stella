#ifndef MYLIBC_H
#define MYLIBC_H

#include "stb_sprintf.h"

#include "common.hpp"
#include "temporary_storage.hpp"


extern "C" {
	void* mlc_memset(void *p, s32 v, u64 n);
	void* mlc_memcpy(void *dst, void const* src, u64 n);
	s32 mlc_memcmp(void const* a, void const* b, u64 n);
	void* mlc_memmove(void *dst, void const* src, u64 n);
	void const* mlc_memchr(void const* p, s32 v, u64 n);
	s32 mlc_strcmp(char const* a, char const* b);
	u64 mlc_strlen(char const* s);
	char const* mlc_strchr(char const* s, s32 c);
	char const* mlc_strstr(char const* a, char const* b);
	char* mlc_strcpy(char *dst, char const* src);
	char* mlc_strncpy(char *dst, char const* src, u64 n);
	s32 mlc_strncmp(char const* a, char const* b, u64 n);
	s64 mlc_strtol(const char* str, char** endptr, s32 base); // TODO: long int?
	f32 mlc_atof(char const* s);
	s32 mlc_toupper(s32 x);
	void mlc_qsort(void* base, u64 num, u64 size, s32 (*compar)(void const*, void const*));
	s32 mlc_sscanf(char const* s, char const* format, ...);
}


#endif