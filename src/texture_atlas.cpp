// NOTE TODO: So, we tried out the traditional texture atlas system
// but actually, surprisingly, failed to get it working artifact-free.
// This was before we did MIP mapping. Obviously, now that we _do_ MIP
// mapping, we can't use this.
//
// We should rewrite this to use OpenGL texture arrays. I've never worked
// with them before but they look simple enough.
//
// Frankly, the reason I haven't already done this is that our Batch_Renderer
// shader (and the chunk shader, as of now) are written to use regular textures.
//
// I could easily convert those shaders (and the supporting C++ code) to use
// array textures, but then they wouldn't support regular textures.
//
// The thing is, I really want to avoid having to do a conditional branch in my
// shaders to check whether we should use a 2D or 3D textures.
// So. Until I stop being stupid and realize a way to do this, uh.. yeah. F.
//
//                      - sci4me, 5/18/20
//

enum {
    UV_TOP_LEFT = 0,
    UV_TOP_RIGHT = 1,
    UV_BOTTOM_RIGHT = 2,
    UV_BOTTOM_LEFT = 3
};

struct Texture_Atlas_Entry {
    struct Texture_Atlas *atlas;
    vec2 uvs[4];
};

struct Texture_Atlas {
    static constexpr GLenum internal_format = GL_RGBA8;
    static constexpr GLenum data_format = GL_RGBA;

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
                vec2(u1, v1),
                vec2(u2, v1),
                vec2(u2, v2),
                vec2(u1, v2)
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