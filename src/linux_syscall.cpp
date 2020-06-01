#include <sys/fcntl.h>
#include <sys/stat.h>

#include "common.hpp"

extern "C" {
	void* syscall0(void*);
	void* syscall1(void*, void*);
	void* syscall2(void*, void*, void*);
	void* syscall3(void*, void*, void*, void*);
	void* syscall4(void*, void*, void*, void*, void*);
	void* syscall5(void*, void*, void*, void*, void*, void*);
	void* syscall6(void*, void*, void*, void*, void*, void*, void*);
}

#define SYSCALL0(a) 					syscall0((void*)a)
#define SYSCALL1(a, b) 					syscall1((void*)a, (void*)b)
#define SYSCALL2(a, b, c)				syscall2((void*)a, (void*)b, (void*)c)
#define SYSCALL3(a, b, c, d)			syscall3((void*)a, (void*)b, (void*)c, (void*)d)
#define SYSCALL4(a, b, c, d, e)			syscall4((void*)a, (void*)b, (void*)c, (void*)d, (void*)e)
#define SYSCALL5(a, b, c, d, e, f)		syscall5((void*)a, (void*)b, (void*)c, (void*)d, (void*)e, (void*)f)
#define SYSCALL6(a, b, c, d, e, f, g)	syscall6((void*)a, (void*)b, (void*)c, (void*)d, (void*)e, (void*)f, (void*)g)

#define SEEK_SET			0
#define SEEK_CUR			1
#define SEEK_END			2

#define MAP_FAILED ((void *) -1)

#define PROT_NONE      0
#define PROT_READ      1
#define PROT_WRITE     2
#define PROT_EXEC      4
#define PROT_GROWSDOWN 0x01000000
#define PROT_GROWSUP   0x02000000

#define MAP_SHARED     0x01
#define MAP_PRIVATE    0x02
#define MAP_FIXED      0x10

#define MAP_TYPE       0x0f
#define MAP_FILE       0x00
#define MAP_ANON       0x20
#define MAP_ANONYMOUS  MAP_ANON
#define MAP_NORESERVE  0x4000
#define MAP_GROWSDOWN  0x0100
#define MAP_DENYWRITE  0x0800
#define MAP_EXECUTABLE 0x1000
#define MAP_LOCKED     0x2000
#define MAP_POPULATE   0x8000
#define MAP_NONBLOCK   0x10000
#define MAP_STACK      0x20000
#define MAP_HUGETLB    0x40000

#define CLOCK_REALTIME            0
#define CLOCK_MONOTONIC           1
#define CLOCK_PROCESS_CPUTIME_ID  2
#define CLOCK_THREAD_CPUTIME_ID   3
#define CLOCK_MONOTONIC_RAW       4
#define CLOCK_REALTIME_COARSE     5
#define CLOCK_MONOTONIC_COARSE    6
#define CLOCK_BOOTTIME            7
#define CLOCK_REALTIME_ALARM      8
#define CLOCK_BOOTTIME_ALARM      9
#define CLOCK_SGI_CYCLE          10
#define CLOCK_TAI                11

#define SYS_read			0
#define SYS_write 			1
#define SYS_open			2
#define SYS_close			3
#define SYS_stat			4
#define SYS_fstat			5
#define SYS_lstat			6
#define SYS_poll			7
#define SYS_lseek			8
#define SYS_mmap			9
#define SYS_mprotect		10
#define SYS_munmap			11

#define SYS_getpid			39
#define SYS_exit 			60
#define SYS_readlink		89
#define SYS_gettid			186
#define SYS_clock_gettime	228
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

void* sc_mmap(void *addr, u64 len, s32 prot, s32 flags, s32 fd, s64 offset) {
	return SYSCALL6(SYS_mmap, addr, len, (s64)prot, (s64)flags, (s64)fd, offset);
}

s32 sc_mprotect(void *addr, u64 len, s32 prot) {
	return (s32)(s64) SYSCALL3(SYS_mprotect, addr, len, (s64)prot);
}

s32 sc_munmap(void *addr, u64 len) {
	return (s32)(s64) SYSCALL2(SYS_munmap, addr, len);
}

s32 sc_getpid() {
	return (s32)(s64) SYSCALL0(SYS_getpid);
}

void sc_exit(s32 code) {
	SYSCALL1(SYS_exit, (s64)code);
}

s64 sc_readlink(char const* path, char *buf, u64 bufsz) {
	return (s64) SYSCALL3(SYS_readlink, path, buf, bufsz);
}

s32 sc_gettid() {
	return (s32)(s64) SYSCALL0(SYS_gettid);
}

s32 sc_clock_gettime(s32 id, struct timespec *ts) {
	return (s32)(s64) SYSCALL2(SYS_clock_gettime, (s64)id, (s64)ts);
}

s32 sc_tgkill(s32 tgid, s32 tid, s32 sig) {
	return (s32)(s64) SYSCALL3(SYS_tgkill, (s64)tgid, (s64)tid, (s64)sig);
}