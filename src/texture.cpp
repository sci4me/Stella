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

inline f32 square(f32 x) { return x * x; } 

inline glm::vec4 rgba255_to_rgba1(u32 c) {
    u8 r = c & 0xFF;
    u8 g = (c >> 8) & 0xFF;
    u8 b = (c >> 16) & 0xFF;
    u8 a = (c >> 24) & 0xFF;
    return {
        (f32)r / 255.0f,
        (f32)g / 255.0f,
        (f32)b / 255.0f,
        (f32)a / 255.0f
    };
}

inline u32 rgba1_to_rgba255(glm::vec4 c) {
    u8 r = (u8) roundf(c.x * 255.0f);
    u8 g = (u8) roundf(c.y * 255.0f);
    u8 b = (u8) roundf(c.z * 255.0f);
    u8 a = (u8) roundf(c.w * 255.0f);
    return a << 24 | b << 16 | g << 8 | r;
}

inline glm::vec4 rgba1_to_linear(glm::vec4 c) {
    return {
        square(c.x),
        square(c.y),
        square(c.z),
        c.w
    };
}

inline glm::vec4 linear_to_rgba1(glm::vec4 c) {
    return {
        sqrtf(c.x),
        sqrtf(c.y),
        sqrtf(c.z),
        c.w
    };
}

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

            auto c = (p00 + p10 + p01 + p11) / 4.0f;

            *dest_pixel++ = rgba1_to_rgba255(linear_to_rgba1(c));
        }

        source_row += 2 * size;
    }
}

void alpha_premultiply(u32 *image, u32 w, u32 h) {
    u32 *pixel = image;
    for(u32 y = 0; y < h; y++) {
        for(u32 x = 0; x < w; x++) {
            auto p = rgba1_to_linear(rgba255_to_rgba1(*pixel));
            p.x *= p.w;
            p.y *= p.w;
            p.z *= p.w;
            *pixel++ = rgba1_to_rgba255(linear_to_rgba1(p));
        }
    }
}

Texture load_texture_from_file(const char *path, bool generate_mipmaps = false) {
    s32 w, h, _n;
    u8 *image = stbi_load(path, &w, &h, &_n, 4);

    if(!image) {
        fprintf(stderr, "Failed to load texture '%s'\n", path);
        exit(1);
    }

    alpha_premultiply((u32*)image, w, h);

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