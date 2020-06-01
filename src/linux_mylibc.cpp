#include "linux_syscall.cpp"

extern "C" {
	void* mlc_memcpy(void *dst, void const* src, u64 n);
	void mlc_free(void* p);

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

	void mlc_free(void *p) {
		if(p == 0) return;

		u64 *x = ((u64*)p) - 1;
		sc_munmap(x, *x);
	}

	void mlc_fwrite(s32 fd, char const* str) {
		// NOTE TODO: Double-check on that +1...
		sc_write(fd, str, mlc_strlen(str) + 1);
	}

	// NOTE: This always adds a null terminator to the end
	// of the data, but that is not included in `len`.
	Buffer read_entire_file(char const* name) {
	    // TODO: make this non-os-dependent!
	    // er.. isolate all os-dependent code.

	    s32 fd = sc_open(name, O_RDONLY);
	    if(fd == -1) return { 0, 0 };

	    struct stat statbuf;
	    sc_fstat(fd, &statbuf);

	    u64 rem = (u64) statbuf.st_size;
	    u8 *data = (u8*) mlc_malloc(rem + 1);

	    u8 *ptr = data;
	    while(rem) {
	        s64 n = sc_read(fd, ptr, rem);
	        
	        if(n == -1) {
	            mlc_free(data);
	            sc_close(fd);
	            return { 0, 0 };
	        }

	        ptr += n;
	        rem -= n;
	    }

	    data[statbuf.st_size] = 0;

	    sc_close(fd);

	    return { data, (u64) statbuf.st_size };
	}

	void mlc_exit(s32 code) {
		sc_exit(code);
	}

	u64 nanotime() {
		struct timespec t;
		sc_clock_gettime(CLOCK_REALTIME, &t);
		return t.tv_sec * 1000000000LLU + t.tv_nsec;
	}
}