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
            glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    void free() {
        glDeleteTextures(1, &id);
    }

    void set_data(void *image, u32 level = 0) {
        u32 denom = (u32)pow(2, level);
        glTextureSubImage2D(id, level, 0, 0, width / denom, height / denom, GL_RGBA, GL_UNSIGNED_BYTE, image);
    }

    void bind(u32 slot) {
        glBindTextureUnit(slot, id);
    }
};

f32 square(f32 x) { return x * x; } 

/*
glm::vec4 rgba255_to_linear(u32 c) {
    u8 r = (c & 0xFF000000) >> 24;
    u8 g = (c & 0x00FF0000) >> 16;
    u8 b = (c & 0x0000FF00) >> 8;
    u8 a = c & 0xFF;
    return {
        square((f32)r / 255.0f),
        square((f32)g / 255.0f),
        square((f32)b / 255.0f),
        (f32)a / 255.0f
    };
}

u32 linear_to_rgba255(glm::vec4 c) {
    u8 r = (u8) (sqrtf(c.x) * 255.0f);
    u8 g = (u8) (sqrtf(c.y) * 255.0f);
    u8 b = (u8) (sqrtf(c.z) * 255.0f);
    u8 a = (u8) (c.w * 255.0f);
    return r << 24 | g << 16 | b << 8 | a;
}
*/

glm::vec4 rgba255_to_linear(u32 c) {
    u8 r = (c & 0xFF000000) >> 24;
    u8 g = (c & 0x00FF0000) >> 16;
    u8 b = (c & 0x0000FF00) >> 8;
    u8 a = c & 0xFF;
    return {
        powf((f32)r / 255.0f, 2.2f),
        powf((f32)g / 255.0f, 2.2f),
        powf((f32)b / 255.0f, 2.2f),
        (f32)a / 255.0f
    };
}

u32 linear_to_rgba255(glm::vec4 c) {
    u8 r = (u8) (powf(c.x, 1.0f/2.2f) * 255.0f);
    u8 g = (u8) (powf(c.y, 1.0f/2.2f) * 255.0f);
    u8 b = (u8) (powf(c.z, 1.0f/2.2f) * 255.0f);
    u8 a = (u8) (c.w * 255.0f);
    return r << 24 | g << 16 | b << 8 | a;
}

void downsample_2x_in_place(u8 *image, u32 size) {
    u32 result_size = size >> 1;

    u32 *dest_pixel = (u32*) image;
    u32 *source_row = (u32*) image;

    for(u32 y = 0; y < result_size; y++) {
        u32 *source_pixel_0 = source_row;
        u32 *source_pixel_1 = source_row + size;
        
        for(u32 x = 0; x < result_size; x++) {
            auto p00 = rgba255_to_linear(*source_pixel_0++);
            auto p10 = rgba255_to_linear(*source_pixel_0++);
            auto p01 = rgba255_to_linear(*source_pixel_1++);
            auto p11 = rgba255_to_linear(*source_pixel_1++);

            auto c = 0.25f * (p00 + p10 + p01 + p11);

            *dest_pixel++ = linear_to_rgba255(c);
        }

        source_row += 2 * size;
    }
}

Texture load_texture_from_file(const char *path, bool generate_mipmaps = false) {
    s32 w, h, _n;
    u8 *image = stbi_load(path, &w, &h, &_n, 4);

    if(!image) {
        fprintf(stderr, "Failed to load texture '%s'\n", path);
        exit(1);
    }

    Texture result;

    u32 levels = 1;
    if(generate_mipmaps) {
        assert((w & (w - 1)) == 0);
        assert((h & (h - 1)) == 0);
        assert(w == h);

        levels = (u32) log2(w) + 1;
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