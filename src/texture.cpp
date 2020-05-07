struct Texture {
    GLuint id;
    u32 width;
    u32 height;

    void init(u32 width, u32 height) {
        this->width = width;
        this->height = height;

        glCreateTextures(GL_TEXTURE_2D, 1, &id);
        glTextureStorage2D(id, 1, GL_RGBA8, width, height);

        glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    void free() {
        glDeleteTextures(1, &id);
    }

    void set_data(void *image) {
        glTextureSubImage2D(id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image);
    }

    void bind(u32 slot) {
        glBindTextureUnit(slot, id);
    }
};

Texture load_texture_from_file(const char *path) {
    s32 w, h, _n;
    u8 *image = stbi_load(path, &w, &h, &_n, 4);

    if(!image) {
        fprintf(stderr, "Failed to load texture '%s'\n", path);
        exit(1);
    }

    Texture result;
    result.init(w, h);
    result.set_data(image);

    stbi_image_free(image);

    return result;
}