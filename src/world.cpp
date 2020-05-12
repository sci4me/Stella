struct Chunk {
    #pragma pack(push, 1)
    struct Vertex {
        glm::vec2 pos;
        glm::vec2 uv;
        s32 tex;
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

    Tile_Type layer0[SIZE][SIZE];
    struct {
        glm::ivec2 key;
        Tile *value;
    } *layer1;

    Vertex_Array vao;
    Vertex_Buffer vbo;
    Index_Buffer ibo;
    u32 textures[MAX_TEXTURE_SLOTS];
    u32 texture_count;

    void init(struct World *world, s32 x, s32 y);
    void free();

    void generate();
    void render();
    void draw();

    rnd_pcg_t make_rng_for_chunk();
};

struct World {
    siv::PerlinNoise noise;

    u32 seed;

    struct {
        glm::ivec2 key;
        struct Chunk *value;
    } *chunks = NULL;

    GLuint chunk_shader;
    GLuint u_textures;
    GLuint u_proj;
    GLuint u_view;

    void init(u32 seed = 1) {
        assert(seed);
        this->seed = seed;
        noise.reseed(seed);

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
            c->init(this, x, y);

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

    u32 render_around(Batch_Renderer *r, glm::vec2 pos, f32 scale, s32 window_width, s32 window_height, glm::mat4 view) {
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

        // TODO: instead of taking `view` as a parameter, we could technically just get it from Batch_Renderer?
        glProgramUniformMatrix4fv(chunk_shader, u_view, 1, GL_FALSE, glm::value_ptr(view));


        glUseProgram(chunk_shader);
        for(s32 i = vp_min_cx; i < vp_max_cx; i++) {
            for(s32 j = vp_min_cy; j < vp_max_cy; j++) {
                Chunk *c = get_chunk(i, j);
                c->draw();
            }
        }
        glUseProgram(0);


        for(s32 i = vp_min_cx; i < vp_max_cx; i++) {
            for(s32 j = vp_min_cy; j < vp_max_cy; j++) {
                Chunk *c = get_chunk(i, j);

                for(u32 k = 0; k < hmlen(c->layer1); k++) {
                    auto p = c->layer1[k].key;
                    auto tile = c->layer1[k].value;

                    s32 l = (i * Chunk::SIZE) + p.x;
                    s32 m = (j * Chunk::SIZE) + p.y;

                    if(l < vp_min_x || m < vp_min_y || l > vp_max_x || m > vp_max_y) continue;

                    r->push_textured_quad(l * TILE_SIZE, m * TILE_SIZE, TILE_SIZE, TILE_SIZE, &tile_textures[(u32) tile->type]);
                }
            }
        }


        return (vp_max_cx - vp_min_cx) * (vp_max_cy - vp_min_cy);
    }
};

void Chunk::init(World *world, s32 x, s32 y) {
    this->world = world;
    this->x = x;
    this->y = y;
    
    vbo.init(MAX_VERTICES * sizeof(Vertex), GL_STATIC_DRAW);
    ibo.init(MAX_INDICES * sizeof(u32), GL_STATIC_DRAW);

    vao.init();
    vao.add_vertex_buffer(vbo, {
        { GL_FLOAT, 2 },
        { GL_FLOAT, 2 },
        { GL_INT, 1 }
    });
    vao.set_index_buffer(ibo);
}

void Chunk::free() {
    vao.free();
    vbo.free();
    ibo.free();
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

            if(m < stone_threshold) layer0[i][j] = TILE_STONE;
            else                    layer0[i][j] = TILE_GRASS;
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

            if(m < coal_threshold) {
                auto tile = (Tile_Ore*) malloc(sizeof(Tile_Ore));
                tile->type = TILE_COAL_ORE;
                tile->count = 1; // TODO

                glm::ivec2 key = {i, j};
                hmput(layer1, key, tile);
            }
        }
    }
}

void Chunk::render() {
    texture_count = 0;

    // TODO don't use the heap for these buffers?
    auto vertices = (Static_Array<Vertex, MAX_VERTICES>*) malloc(sizeof(Static_Array<Vertex, MAX_VERTICES>));
    vertices->clear();
    auto indices = (Static_Array<u32, MAX_INDICES>*) malloc(sizeof(Static_Array<u32, MAX_INDICES>));
    indices->clear();

    rnd_pcg_t l0rot = make_rng_for_chunk();

    static const glm::vec2 uvs[4][4] = {
        { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } },
        { { 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f } },
        { { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
        { { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f } }
    };

    for(s32 i = 0; i < SIZE; i++) {
        for(s32 j = 0; j < SIZE; j++) {
            auto tile = layer0[i][j];

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

            auto rot = rnd_pcg_range(&l0rot, 0, 3);

            u32 tl = vertices->push({ {k, l}, uvs[rot][0], tex_index });
            u32 tr = vertices->push({ {k + TILE_SIZE, l}, uvs[rot][1], tex_index });
            u32 br = vertices->push({ {k + TILE_SIZE, l + TILE_SIZE}, uvs[rot][2], tex_index });
            u32 bl = vertices->push({ {k, l + TILE_SIZE}, uvs[rot][3], tex_index });

            indices->push(tl);
            indices->push(tr);
            indices->push(br);
            indices->push(br);
            indices->push(bl);
            indices->push(tl);
        }
    }

    assert(vertices->count == MAX_VERTICES);
    assert(indices->count == MAX_INDICES);

    vbo.set_data(&vertices->data, MAX_VERTICES * sizeof(Vertex));
    ibo.set_data(&indices->data, MAX_INDICES * sizeof(u32));

    ::free(vertices);
    ::free(indices);
}

void Chunk::draw() {
    vao.bind();

    for(u32 i = 0; i < texture_count; i++)
        glBindTextureUnit(i, textures[i]);

    glDrawElements(GL_TRIANGLES, MAX_INDICES, GL_UNSIGNED_INT, 0);

    for(u32 i = 0; i < texture_count; i++)
        glBindTextureUnit(i, 0);

    vao.unbind();
}

rnd_pcg_t Chunk::make_rng_for_chunk() {
    rnd_pcg_t rand;
    rnd_pcg_seed(&rand, x * world->seed + y * world->seed);
    return rand;
}