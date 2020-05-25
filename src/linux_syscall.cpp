#include <sys/fcntl.h>
#include <sys/stat.h>

extern "C" {
	void* syscall0(void*);
	void* syscall1(void*, void*);
	void* syscall2(void*, void*, void*);
	void* syscall3(void*, void*, void*, void*);
	void* syscall4(void*, void*, void*, void*, void*);
	void* syscall5(void*, void*, void*, void*, void*, void*);
}

#define SYSCALL0(a) 				syscall0((void*)a)
#define SYSCALL1(a, b) 				syscall1((void*)a, (void*)b)
#define SYSCALL2(a, b, c)			syscall2((void*)a, (void*)b, (void*)c)
#define SYSCALL3(a, b, c, d)		syscall3((void*)a, (void*)b, (void*)c, (void*)d)
#define SYSCALL4(a, b, c, d, e)		syscall4((void*)a, (void*)b, (void*)c, (void*)d, (void*)e)
#define SYSCALL5(a, b, c, d, e, f)	syscall5((void*)a, (void*)b, (void*)c, (void*)d, (void*)e, (void*)f)

#define STDIN 				0
#define STDOUT				1
#define STDERR				2

#define SEEK_SET			0
#define SEEK_CUR			1
#define SEEK_END			2

#define SYS_read			0
#define SYS_write 			1
#define SYS_open			2
#define SYS_close			3
#define SYS_stat			4
#define SYS_fstat			5
#define SYS_lstat			6
#define SYS_lseek			8

#define SYS_getpid			39
#define SYS_exit 			60
#define SYS_gettid			186
#define SYS_tgkill			234

// TODO: Force these to be inlined!

s64 sc_read(s32 fd, void *data, u64 n) {
	return (s64) SYSCALL3(SYS_read, (s64)fd, data, n);
}

s64 sc_write(s32 fd, void const* data, u64 n) {
	return (s64) SYSCALL3(SYS_write, (s64)fd, data, n);
}

s32 sc_open(char const* path, s32 flags) {
	return (s32)(s64) SYSCALL2(SYS_open, path, (s64)flags);
}

s32 sc_open(char const* path, s32 flags, u32 mode) {
	return (s32)(s64) SYSCALL3(SYS_open, path, (s64)flags, (u64)mode);
}

s32 sc_close(s32 fd) {
	return (s32)(s64) SYSCALL1(SYS_close, (s64)fd);
}

s32 sc_stat(char const* path, struct stat *statbuf) {
	return (s32)(s64) SYSCALL2(SYS_stat, path, statbuf);
}

s32 sc_fstat(s32 fd, struct stat *statbuf) {
	return (s32)(s64) SYSCALL2(SYS_fstat, (s64)fd, statbuf);
}

s32 sc_lstat(char const* path, struct stat *statbuf) {
	return (s32)(s64) SYSCALL2(SYS_lstat, path, statbuf);
}

s64 sc_lseek(s32 fd, s64 offset, s32 whence) {
	return (s64) SYSCALL3(SYS_lseek, (s64)fd, offset, (s64)whence);
}

s32 sc_getpid() {
	return (s32)(s64) SYSCALL0(SYS_getpid);
}

void sc_exit(s32 code) {
	SYSCALL1(SYS_exit, (s64)code);
}

s32 sc_gettid() {
	return (s32)(s64) SYSCALL0(SYS_gettid);
}

s32 sc_tgkill(s32 tgid, s32 tid, s32 sig) {
	return (s32)(s64) SYSCALL3(SYS_tgkill, (s64)tgid, (s64)tid, (s64)sig);
}