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
        if(!p) return;
        
        u64 *s = ((u64*)p) - 1;
        VirtualFree(p, *s, MEM_RELEASE);
    }

    void mlc_fwrite(s32 fd, char const* str) {
        HANDLE stream = 0;

        switch(fd) {
            case STDOUT:
                stream = GetStdHandle(STD_OUTPUT_HANDLE);
                break;
            case STDERR:
                stream = GetStdHandle(STD_ERROR_HANDLE);
                break;
            default:
                assert(0);
                break;
        }

        if(!stream) return;

        DWORD _n;
        WriteConsoleA(stream, str, mlc_strlen(str), &_n, 0);
    }


    Buffer read_entire_file(char const* name) {
        HANDLE fh = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
        if(!fh) {
            return { 0, 0 };
        }

        DWORD size;
        assert(GetFileSizeEx(fh, (PLARGE_INTEGER) &size));

        u8 *data = (u8*) mlc_alloc(size + 1);
        DWORD n_read;
        assert(ReadFile(fh, data, size, &n_read, 0));
        assert(n_read == size);

        data[size] = 0;

        CloseHandle(fh);

        return { data, (u64)size };
    }

    void mlc_exit(s32 code) {
        ExitProcess(code);
    }

    u64 nanotime() {
        // TODO
    }
}