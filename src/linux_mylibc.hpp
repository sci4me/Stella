#ifndef LINUX_MYLIBC_H
#define LINUX_MYLIBC_H

extern "C" {
	void* mlc_malloc(u64 n);
	void* mlc_calloc(u64 n, u64 s);
	void* mlc_realloc(void *p, u64 n);
	void mlc_free(void *p);
	void mlc_fwrite(s32 fd, char const* str);
	Buffer read_entire_file(char const* name);
	void mlc_exit(s32 code);
	u64 nanotime();
}

#endif