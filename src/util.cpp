#define array_length(a) ((sizeof(a))/(sizeof(a[0])))


void tprintf(char const* fmt, ...) {
    va_list args, args2;
    va_start(args, fmt);
    va_copy(args2, args);
    s32 len = stbsp_vsnprintf(0, 0, fmt, args);
    va_end(args);

    char *buf = (char*) talloc(len);

    va_start(args2, fmt);
    stbsp_vsprintf(buf, fmt, args2);
    va_end(args2);

    buf[len] = 0;

    sc_write(1, buf, len + 1);
}


char* read_entire_file(char *name) {
    // TODO: make this non-os-dependent!
    // er.. isolate all os-dependent code.

    s32 fd = sc_open(name, O_RDONLY);
    if(fd == -1) return 0;

    struct stat statbuf;
    sc_fstat(fd, &statbuf);

    u64 rem = (u64) statbuf.st_size;
    char *data = (char*) mlc_malloc(rem);

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

    sc_close(fd);

    /*
    FILE *fp = fopen(name, "rb");
    if (!fp) {
        return 0;
    }

    fseek(fp, 0L, SEEK_END);
    u64 size = ftell(fp);
    rewind(fp);

    char *code = (char*) malloc(size + 1);

    assert(fread(code, sizeof(char), size, fp) == size);
    code[size] = 0;

    return code;
    */
    return 0;
}


void dump_gl_extensions() {
    tprintf("  GL_EXTENSIONS\n");
        
    s32 n_ext;
    glGetIntegerv(GL_NUM_EXTENSIONS, &n_ext);
    
    // char const** exts = (char const**) talloc(n_ext * sizeof(char const*));
    char const** exts = (char const**) alloca(n_ext * sizeof(char const*));

    u32 max_len = 0;
    for(s32 i = 0; i < n_ext; i++) {
        char const *ext = (char const*) glGetStringi(GL_EXTENSIONS, i);

        exts[i] = ext;

        u32 len = mlc_strlen(ext);
        if(len > max_len) {
            max_len = len;
        }
    }

    constexpr u32 cols = 2;
    u32 column_length = max_len + 2;
    u32 column = 0;
    for(u32 i = 0; i < n_ext; i++) {
        char const* ext = exts[i];
        u32 padding = column_length - mlc_strlen(ext);

        tprintf("%s", ext);
        for(u32 j = 0; j < padding; j++) tprintf(" ");

        column++;
        if(column > (cols - 1)) {
            column = 0;
            tprintf("\n");
        }
    }

    if(column <= (cols - 1)) tprintf("\n");
}

void dump_gl_info() {
    // TODO: some kind of logging!!!

    tprintf("OpenGL Info:\n");

    GLint major, minor; 
    glGetIntegerv(GL_MAJOR_VERSION, &major); 
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    tprintf("  GL_MAJOR_VERSION              %d\n", major);
    tprintf("  GL_MINOR_VERSION              %d\n", minor);

    tprintf("  GL_VENDOR                     %s\n", glGetString(GL_VENDOR));
    tprintf("  GL_RENDERER                   %s\n", glGetString(GL_RENDERER));
    tprintf("  GL_VERSION                    %s\n", glGetString(GL_VERSION));
    tprintf("  GL_SHADING_LANGUAGE_VERSION   %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
}


constexpr u32 rotl32(u32 x, u32 r) {
    return (x << r) | (x >> (32 - r));
}


static constexpr vec2 QUAD_UVS[4][4] = {
    { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } },
    { { 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f } },
    { { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f } }
};