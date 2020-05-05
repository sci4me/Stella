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

GLuint load_texture(const char *path) {
    s32 w, h, n;
    u8 *image = stbi_load(path, &w, &h, &n, 0);

    if(!image) {
        fprintf(stderr, "Failed to load texture '%s'\n", path);
        exit(1);
    }

    GLenum internal_format;
    GLenum data_format;
    switch(n) {
        case 3:
            internal_format = GL_RGB8;
            data_format = GL_RGB;
            break;
        case 4:
            internal_format = GL_RGBA8;
            data_format = GL_RGBA;
            break;
        default:
            assert(0);
    }

    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);

    glTextureStorage2D(texture, 1, internal_format, w, h);
    glTextureSubImage2D(texture, 0, 0, 0, w, h, data_format, GL_UNSIGNED_BYTE, image);

    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return texture;
}

f32 randf32() {
    // TODO REMOVEME
    return (f32)rand() / (f32)RAND_MAX;
}

f32 clampf(f32 x, f32 min, f32 max) {
    if(x < min) return min;
    if(x > max) return max;
    return x;
}