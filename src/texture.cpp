struct Texture {
    GLuint id;
    u32 width;
    u32 height;

    void init(u32 width, u32 height, u32 levels = 1) {
        assert(levels);

        this->width = width;
        this->height = height;

        glCreateTextures(GL_TEXTURE_2D, 1, &id);
        glTextureStorage2D(id, levels, GL_RGBA8, width, height);

        if(levels > 1) {
            glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        } else {
            glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    void deinit() {
        glDeleteTextures(1, &id);
    }

    void set_data(void *image, u32 level = 0) {
        u32 denom = (u32)powf32(2, level);
        glTextureSubImage2D(id, level, 0, 0, width / denom, height / denom, GL_RGBA, GL_UNSIGNED_BYTE, image);
    }

    void bind(u32 slot) {
        glBindTextureUnit(slot, id);
    }
};

void downsample_2x_in_place(u8 *image, u32 size) {
    u32 result_size = size >> 1;

    u32 *dest_pixel = (u32*) image;
    u32 *source_row = (u32*) image;

    for(u32 y = 0; y < result_size; y++) {
        u32 *source_pixel_0 = source_row;
        u32 *source_pixel_1 = source_row + size;
        
        for(u32 x = 0; x < result_size; x++) {
            auto p00 = rgba1_to_linear(rgba255_to_rgba1(*source_pixel_0++));
            auto p10 = rgba1_to_linear(rgba255_to_rgba1(*source_pixel_0++));
            auto p01 = rgba1_to_linear(rgba255_to_rgba1(*source_pixel_1++));
            auto p11 = rgba1_to_linear(rgba255_to_rgba1(*source_pixel_1++));

            f32 s = 1.0f / 4.0f;
            auto c = (p00 + p10 + p01 + p11) * s;

            *dest_pixel++ = rgba1_to_rgba255(linear_to_rgba1(c));
        }

        source_row += 2 * size;
    }
}

void alpha_premultiply_in_place(u32 *image, u32 w, u32 h) {
    // NOTE: We could definitely do this with SIMD.
    // NOT necessary. AT ALL. Not even a little bit.
    // Hell, eventually this would really just be something
    // we do at build time anyway, so, yeah. But. Yknow.
    // Maybe if I'm bored sometime.

    u32 *pixel = image;
    for(u32 i = 0; i < w * h; i++) {
        *pixel++ = rgba1_to_rgba255(alpha_premultiply(rgba255_to_rgba1(*pixel)));
    }
}

Texture load_texture_from_file(char const* type, char const* name, bool generate_mipmaps = false) {
    char path[256];
    stbsp_snprintf(path, sizeof(path), "assets/textures/%s/%s.png", type, name);

    Buffer file = read_entire_file(path);

    s32 w, h, _n;
    u8 *image = stbi_load_from_memory(file.data, file.len, &w, &h, &_n, 4);

    if(!image) {
        tfprintf(STDERR, "Failed to load texture '%s'\n", path);
        mlc_exit(1);
    }

    mlc_free(file.data);

    alpha_premultiply_in_place((u32*)image, w, h);

    Texture result;

    u32 levels = 1;
    if(generate_mipmaps) {
        assert((w & (w - 1)) == 0);
        assert((h & (h - 1)) == 0);
        assert(w == h);

        levels = (u32) log2f32(w) + 1;
    }

    result.init(w, h, levels);

    result.set_data(image, 0);
    for(u32 level = 1; level < levels; level++) {
        downsample_2x_in_place(image, w);
        w >>= 1;
        result.set_data(image, level);
    }

    stbi_image_free(image);

    return result;
}