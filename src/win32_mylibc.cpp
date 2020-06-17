extern "C" {
    void* mlc_memcpy(void *dst, void const* src, u64 n);
    void mlc_free(void* p);

    void* mlc_alloc(u64 n) {

    }

    void mlc_free(void *p) {

    }

    void mlc_fwrite(s32 fd, char const* str) {

    }


    Buffer read_entire_file(char const* name) {
        
    }

    void mlc_exit(s32 code) {

    }

    u64 nanotime() {

    }
}