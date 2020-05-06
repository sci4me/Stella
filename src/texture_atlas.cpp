enum {
    UV_TOP_LEFT = 0,
    UV_TOP_RIGHT = 1,
    UV_BOTTOM_RIGHT = 2,
    UV_BOTTOM_LEFT = 3
};

struct Texture_Atlas_Entry {
    struct Texture_Atlas *atlas;
    glm::vec2 uvs[4];
};

struct Texture_Atlas {
    static const GLenum internal_format = GL_RGBA8;
    static const GLenum data_format = GL_RGBA;

    GLuint id;

    u32 sprite_width;
    u32 sprite_height;
    u32 columns;
    u32 rows;

    Texture_Atlas_Entry *entries;

    void init(u32 sprite_width, u32 sprite_height, u32 columns, u32 rows) {
        this->sprite_width = sprite_width;
        this->sprite_height = sprite_height;
        this->columns = columns;
        this->rows = rows;
        this->entries = nullptr;

        u32 atlas_width = sprite_width * columns;
        u32 atlas_height = sprite_height * rows;

        glCreateTextures(GL_TEXTURE_2D, 1, &id);
        glTextureStorage2D(id, 1, internal_format, atlas_width, atlas_height);
        
        glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    void free() {
        glDeleteTextures(1, &id);
    }

    u32 add(void *image) {
        u32 index = arrlen(entries);
        assert(index < (columns * rows));

        u32 col = index % columns;
        u32 row = index / columns;
        u32 x = col * sprite_width;
        u32 y = row * sprite_height;
        glTextureSubImage2D(id, 0, x, y, sprite_width, sprite_height, data_format, GL_UNSIGNED_BYTE, image);

        u32 atlas_width = sprite_width * columns;
        u32 atlas_height = sprite_height * rows;
        f32 u1 = ((f32) x) / ((f32) atlas_width);
        f32 v1 = ((f32) y) / ((f32) atlas_height);
        f32 u2 = ((f32) (x + sprite_width)) / ((f32) atlas_width);
        f32 v2 = ((f32) (y + sprite_height)) / ((f32) atlas_height);
        Texture_Atlas_Entry entry = {
            this,
            {
                glm::vec2(u1, v1),
                glm::vec2(u2, v1),
                glm::vec2(u2, v2),
                glm::vec2(u1, v2)
            }
        };
        arrput(entries, entry);

        return index;
    }

    u32 add(const char *path) {
        s32 w, h, _n;
        u8 *image = stbi_load(path, &w, &h, &_n, 4);
        assert(image); // TODO?
        u32 index = add(image);
        stbi_image_free(image);
        return index;
    }
};