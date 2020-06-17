extern "C" {
    void mlc_free(void* p);

    void* mlc_alloc(u64 n) {
        n += sizeof(u64); // NOTE: This is so we can store the size.

        u64 r = n % PAGE_SIZE;
        if(r > 0) {
            // NOTE: Align to page boundary.
            n += PAGE_SIZE - r;
        }

        void *ptr = VirtualAlloc(NULL, n, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if(ptr == 0) return 0;

        *((u64*)ptr) = n;
        return ((u64*)ptr) + 1;
    }

    void mlc_free(void *p) {
        u64 *s = ((u64*)p) - 1;
        VirtualFree(p, *s, MEM_RELEASE);
    }

    void mlc_fwrite(s32 fd, char const* str) {
        // TODO
    }


    Buffer read_entire_file(char const* name) {
        // TODO   
    }

    void mlc_exit(s32 code) {
        ExitProcess(code);
    }

    u64 nanotime() {
        // TODO
    }
}