#define array_length(a) ((sizeof(a))/(sizeof(a[0])))

char* read_entire_file(char *name) {
    FILE *fp = fopen(name, "rb");
    if (!fp) {
        return 0;
    }

    fseek(fp, 0L, SEEK_END);
    u32 size = ftell(fp);
    rewind(fp);

    char *code = (char*) malloc(size + 1);

    // TODO: use fread
    for (u32 i = 0; i < size; i++) {
        code[i] = fgetc(fp);
    }
    code[size] = 0;

    return code;
}

void dump_gl_info() {
    // TODO: some kind of logging!!!

    printf("OpenGL Info:\n");
    printf("  GL_VENDOR                     %s\n", glGetString(GL_VENDOR));
    printf("  GL_RENDERER                   %s\n", glGetString(GL_RENDERER));
    printf("  GL_VERSION                    %s\n", glGetString(GL_VERSION));
    printf("  GL_SHADING_LANGUAGE_VERSION   %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));


    GLint major, minor; 
    glGetIntegerv(GL_MAJOR_VERSION, &major); 
    glGetIntegerv(GL_MINOR_VERSION, &minor); 
    printf("  GL_MAJOR_VERSION              %d\n", major);
    printf("  GL_MINOR_VERSION              %d\n", minor);
}