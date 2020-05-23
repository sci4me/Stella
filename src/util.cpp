#define array_length(a) ((sizeof(a))/(sizeof(a[0])))


char* read_entire_file(char *name) {
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
}


void dump_gl_extensions() {
    printf("  GL_EXTENSIONS\n");
        
    s32 n_ext;
    glGetIntegerv(GL_NUM_EXTENSIONS, &n_ext);
    
    char const** exts = (char const**) talloc(n_ext * sizeof(char const*));

    u32 max_len = 0;
    for(s32 i = 0; i < n_ext; i++) {
        char const *ext = (char const*) glGetStringi(GL_EXTENSIONS, i);

        exts[i] = ext;

        u32 len = strlen(ext);
        if(len > max_len) {
            max_len = len;
        }
    }

    constexpr u32 cols = 2;
    u32 column_length = max_len + 2;
    u32 column = 0;
    for(u32 i = 0; i < n_ext; i++) {
        char const* ext = exts[i];
        u32 padding = column_length - strlen(ext);

        printf("%s", ext);
        for(u32 j = 0; j < padding; j++) printf(" ");

        column++;
        if(column > (cols - 1)) {
            column = 0;
            printf("\n");
        }
    }

    if(column <= (cols - 1)) printf("\n");
}

void dump_gl_info() {
    // TODO: some kind of logging!!!

    printf("OpenGL Info:\n");

    GLint major, minor; 
    glGetIntegerv(GL_MAJOR_VERSION, &major); 
    glGetIntegerv(GL_MINOR_VERSION, &minor); 
    printf("  GL_MAJOR_VERSION              %d\n", major);
    printf("  GL_MINOR_VERSION              %d\n", minor);

    printf("  GL_VENDOR                     %s\n", glGetString(GL_VENDOR));
    printf("  GL_RENDERER                   %s\n", glGetString(GL_RENDERER));
    printf("  GL_VERSION                    %s\n", glGetString(GL_VERSION));
    printf("  GL_SHADING_LANGUAGE_VERSION   %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
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