#define TILE_SIZE 32.0f

enum TileType : u8 {
    TILE_NONE,
    TILE_STONE,
    TILE_GRASS,
    TILE_COAL_ORE,

    N_TILE_TYPES
};

// TODO: Convert this to a texture atlas once
//       we figure out how to fix the artifacts...
Texture tile_textures[N_TILE_TYPES];

void load_tile_textures() {
    constexpr bool MIPS = true;

    tile_textures[TILE_STONE]       = load_texture_from_file("res/textures/stone.png", MIPS);
    tile_textures[TILE_GRASS]       = load_texture_from_file("res/textures/grass.png", MIPS);
    tile_textures[TILE_COAL_ORE]    = load_texture_from_file("res/textures/coal_ore.png", MIPS); 
}

struct Chunk {
    #pragma pack(push, 1)
    struct Vertex {
        glm::vec2 pos;
        glm::vec2 uv;
        s32 tex;
        f32 uv_rotation;
    };
    #pragma pack(pop)

    static const s32 SIZE = 64; // must be a power of 2!
    static const s32 LAYERS = 3;

    static const u32 MAX_VERTICES = SIZE * SIZE * 4;
    static const u32 MAX_INDICES = SIZE * SIZE * 6;
    static const u32 MAX_TEXTURE_SLOTS = 16; // TODO

    struct World *world;
    s32 x;
    s32 y;
    TileType tiles[SIZE][SIZE][LAYERS];

    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    u32 textures[MAX_TEXTURE_SLOTS];
    u32 texture_count;

    void init();
    void free();

    void generate();
    void render();
    void draw();
};

struct World {
    siv::PerlinNoise noise;

    struct {
        glm::ivec2 key;
        struct Chunk *value;
    } *chunks = NULL;

    GLuint chunk_shader;
    GLuint u_textures;
    GLuint u_proj;
    GLuint u_view;

    void init() {
        chunk_shader = load_shader_program("chunk", VERTEX_SHADER | FRAGMENT_SHADER);

        u_textures = glGetUniformLocation(chunk_shader, "u_textures");
        u_proj = glGetUniformLocation(chunk_shader, "u_proj");
        u_view = glGetUniformLocation(chunk_shader, "u_view");

        s32 samplers[Chunk::MAX_TEXTURE_SLOTS];
        for(s32 i = 0; i < Chunk::MAX_TEXTURE_SLOTS; i++) 
            samplers[i] = i;
        glProgramUniform1iv(chunk_shader, u_textures, Chunk::MAX_TEXTURE_SLOTS, samplers);
    }

    void free() {
        glDeleteProgram(chunk_shader);

        for(u32 i = 0; i < hmlen(chunks); i++) {
            ::free(chunks[i].value);
        }
        hmfree(chunks);
    }

    void set_projection(glm::mat4 proj) {
        glProgramUniformMatrix4fv(chunk_shader, u_proj, 1, GL_FALSE, glm::value_ptr(proj));
    }

    Chunk* get_chunk(s32 x, s32 y) {
        glm::ivec2 key = {x, y};
        s64 i = hmgeti(chunks, key);

        if(i == -1) {
            Chunk *c = (Chunk*) calloc(1, sizeof(Chunk));
            c->world = this;
            c->x = x;
            c->y = y;
            c->init();

            c->generate();
            c->render();

            hmput(chunks, key, c);
            
            return c;
        }   

        return chunks[i].value;
    }

    Chunk* get_chunk_containing(s32 x, s32 y) {
        s32 cx = (s32)floor(x / (f32)Chunk::SIZE);
        s32 cy = (s32)floor(y / (f32)Chunk::SIZE);
        return get_chunk(cx, cy);
    }

    TileType get_tile(s32 x, s32 y, s32 layer) {
        Chunk *c = get_chunk_containing(x, y);
        return c->tiles[x & (Chunk::SIZE-1)][y & (Chunk::SIZE-1)][layer];
    }

    void set_tile(s32 x, s32 y, s32 layer, TileType type) {
        assert(type != N_TILE_TYPES);
        Chunk *c = get_chunk_containing(x, y);
        c->tiles[x & (Chunk::SIZE-1)][y & (Chunk::SIZE-1)][layer] = type;
    }

    u32 render_around(Batch_Renderer *r, glm::vec2 pos, f32 scale, s32 window_width, s32 window_height) {
        f32 x = pos.x * scale;
        f32 y = pos.y * scale;

        s32 half_window_width = window_width / 2;
        s32 half_window_height = window_height / 2;

        s32 vp_min_x = (s32) floor(((x - half_window_width) / scale) / TILE_SIZE);
        s32 vp_min_y = (s32) floor(((y - half_window_height) / scale) / TILE_SIZE);
        s32 vp_max_x = (s32) ceil(((x + half_window_width) / scale) / TILE_SIZE);
        s32 vp_max_y = (s32) ceil(((y + half_window_height) / scale) / TILE_SIZE);

        s32 vp_min_cx = (s32) floor((f32)vp_min_x / (f32)Chunk::SIZE);
        s32 vp_min_cy = (s32) floor((f32)vp_min_y / (f32)Chunk::SIZE);
        s32 vp_max_cx = (s32) ceil((f32)vp_max_x / (f32)Chunk::SIZE);
        s32 vp_max_cy = (s32) ceil((f32)vp_max_y / (f32)Chunk::SIZE);

        glm::mat4 view = glm::translate(
            glm::scale(
                glm::mat4(1.0f),
                glm::vec3(scale, scale, 1.0f)
            ),
            glm::vec3((window_width / 2 / scale) - pos.x, (window_height / 2 / scale) - pos.y, 0.0f)
        );
        glProgramUniformMatrix4fv(chunk_shader, u_view, 1, GL_FALSE, glm::value_ptr(view));

        for(s32 layer = 0; layer < Chunk::LAYERS; layer++) {
            if(layer == 0) {
                glUseProgram(chunk_shader);
                for(s32 i = vp_min_cx; i < vp_max_cx; i++) {
                    for(s32 j = vp_min_cy; j < vp_max_cy; j++) {
                        Chunk *c = get_chunk(i, j);
                        c->draw();
                    }
                }
                glUseProgram(0);
            } else {
                for(s32 i = vp_min_cx; i < vp_max_cx; i++) {
                    for(s32 j = vp_min_cy; j < vp_max_cy; j++) {
                        Chunk *c = get_chunk(i, j);

                        for(s32 k = 0; k < Chunk::SIZE; k++) {
                            for(s32 l = 0; l < Chunk::SIZE; l++) {
                                s32 m = (i * Chunk::SIZE) + k;
                                s32 n = (j * Chunk::SIZE) + l;

                                if(m < vp_min_x || n < vp_min_y || m > vp_max_x || n > vp_max_y) continue;

                                auto type = c->tiles[k][l][layer];
                                if(type != TILE_NONE) {
                                    r->push_textured_quad(m * TILE_SIZE, n * TILE_SIZE, TILE_SIZE, TILE_SIZE, &tile_textures[(u32)type]);
                                }
                            }
                        }
                    }
                }
            }
        }

        return (vp_max_cx - vp_min_cx) * (vp_max_cy - vp_min_cy);
    }
};

void Chunk::init() {  
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES*sizeof(Vertex), 0, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, pos));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, uv));
    glVertexAttribIPointer(2, 1, GL_INT, sizeof(Vertex), (void*) offsetof(Vertex, tex));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES*sizeof(u32), 0, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Chunk::free() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
}

void Chunk::generate() {
    constexpr f32 stone_frequency = 70.0f;
    constexpr f32 stone_threshold = 0.4f;

    for(s32 i = 0; i < SIZE; i++) {
        for(s32 j = 0; j < SIZE; j++) {
            f32 m = world->noise.noise2D_0_1(
                ((f32) ((x * SIZE) + i)) / stone_frequency, 
                ((f32) ((y * SIZE) + j)) / stone_frequency
            );

            if(m < stone_threshold) tiles[i][j][0] = TILE_STONE;
            else                    tiles[i][j][0] = TILE_GRASS;
        }
    }

    constexpr f32 coal_frequency = 100.0f;
    constexpr f32 coal_threshold = 0.25f;

    for(s32 i = 0; i < SIZE; i++) {
        for(s32 j = 0; j < SIZE; j++) {
            f32 m = world->noise.noise2D_0_1(
                ((f32) ((x * SIZE) + i)) / coal_frequency, 
                ((f32) ((y * SIZE) + j)) / coal_frequency
            );

            if(m < coal_threshold) tiles[i][j][1] = TILE_COAL_ORE;
        }
    }
}

glm::vec2 rotateUV(glm::vec2 uv, f32 rotation) {
    f32 mid = 0.5f;
    return {
        cosf(rotation) * (uv.x - mid) + sinf(rotation) * (uv.y - mid) + mid,
        cosf(rotation) * (uv.y - mid) - sinf(rotation) * (uv.x - mid) + mid
    };
}

void Chunk::render() {
    texture_count = 0;

    // TODO don't use the heap for these buffers?
    Vertex *vertices = (Vertex*) calloc(MAX_VERTICES, sizeof(Vertex));
    u32 *indices = (u32*) calloc(MAX_INDICES, sizeof(u32));
    u32 vertex_count = 0;
    u32 index_count = 0;

    rnd_pcg_t rand;
    rnd_pcg_seed(&rand, x * y);

    for(s32 i = 0; i < SIZE; i++) {
        for(s32 j = 0; j < SIZE; j++) {
            auto tile = tiles[i][j][0];

            auto texture = tile_textures[tile].id;
            s32 tex_index = 0;
            bool texture_found = false;
            for(u32 t = 0; t < texture_count; t++) {
                if(textures[t] == texture) {
                    tex_index = t;
                    texture_found = true;
                    break;
                }
            }
            if(!texture_found) {
                if(texture_count < MAX_TEXTURE_SLOTS) {
                    textures[texture_count] = texture;
                    tex_index = texture_count;
                    texture_count++;
                } else {
                    assert(0); // TODO
                }
            }

            f32 k = ((x * SIZE) + i) * TILE_SIZE;
            f32 l = ((y * SIZE) + j) * TILE_SIZE;

            // NOTE: We are using `rnd_pcg_range` instead of `rnd_pcg_nextf` on purpose!
            // They end up giving us a totally different distribution, which makes sense
            // I guess. But, well, I think what we get from `rnd_pcg_range` looks better.
            //              - sci4me, 5/9/20
            f32 uv_rotation = PI * 2.0f * ((f32)rnd_pcg_range(&rand, 0, 3) / 4.0f);
            assert(uv_rotation >= 0.0f && uv_rotation <= PI * 2.0f);

            u32 tl = vertex_count++; vertices[tl] = { {k, l}, rotateUV({0.0f, 0.0f}, uv_rotation), tex_index };
            u32 tr = vertex_count++; vertices[tr] = { {k + TILE_SIZE, l}, rotateUV({1.0f, 0.0f}, uv_rotation), tex_index };
            u32 br = vertex_count++; vertices[br] = { {k + TILE_SIZE, l + TILE_SIZE}, rotateUV({1.0f, 1.0f}, uv_rotation), tex_index };
            u32 bl = vertex_count++; vertices[bl] = { {k, l + TILE_SIZE}, rotateUV({0.0f, 1.0f}, uv_rotation), tex_index };

            indices[index_count++] = tl;
            indices[index_count++] = tr;
            indices[index_count++] = br;
            indices[index_count++] = br;
            indices[index_count++] = bl;
            indices[index_count++] = tl;
        }
    }

    assert(vertex_count == MAX_VERTICES);
    assert(index_count == MAX_INDICES);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_VERTICES * sizeof(Vertex), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, MAX_INDICES * sizeof(u32), indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    ::free(vertices);
    ::free(indices);
}

void Chunk::draw() {
    glBindVertexArray(vao);

    for(u32 i = 0; i < texture_count; i++)
        glBindTextureUnit(i, textures[i]);

    glDrawElements(GL_TRIANGLES, MAX_INDICES, GL_UNSIGNED_INT, 0);

    for(u32 i = 0; i < texture_count; i++)
        glBindTextureUnit(i, 0);

    glBindVertexArray(0);
}