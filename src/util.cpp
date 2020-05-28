#define array_length(a) ((sizeof(a))/(sizeof(a[0])))


char* read_entire_file(char *name) {
    // TODO: make this non-os-dependent!
    // er.. isolate all os-dependent code.

    s32 fd = sc_open(name, O_RDONLY);
    if(fd == -1) return 0;

    struct stat statbuf;
    sc_fstat(fd, &statbuf);

    u64 rem = (u64) statbuf.st_size;
    char *data = (char*) mlc_malloc(rem + 1);

    char *ptr = data;
    while(rem) {
        s64 n = sc_read(fd, ptr, rem);
        
        if(n == -1) {
            mlc_free(data);
            sc_close(fd);
            return 0;
        }

        ptr += n;
        rem -= n;
    }

    data[statbuf.st_size] = 0;

    sc_close(fd);

    return data;
}


static constexpr vec2 QUAD_UVS[4][4] = {
    { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } },
    { { 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f } },
    { { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f } }
};