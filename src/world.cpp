struct Chunk {
    #pragma pack(push, 1)
    struct Vertex {
        vec2 pos;
        vec2 uv;
        s32 tex;
    };
    #pragma pack(pop)

    struct Tile_Map {
        ivec2 key;
        Tile *value;
    };

    static constexpr s32 SIZE = 64; // must be a power of 2!
    static constexpr s32 LAYERS = 3;

    static constexpr u32 MAX_VERTICES = SIZE * SIZE * 4;
    static constexpr u32 MAX_INDICES = SIZE * SIZE * 6;
    static constexpr u32 MAX_TEXTURE_SLOTS = 16; // TODO

    struct World *world;
    
    s32 x;
    s32 y;

    Tile_Type layer0[SIZE][SIZE];
    Tile_Map *layer1;
    Tile_Map *layer2;

    Vertex_Array vao;
    Vertex_Buffer vbo;
    Index_Buffer ibo;
    Slot_Allocator<GLuint, MAX_TEXTURE_SLOTS> textures;

    void init(struct World *world, s32 x, s32 y);
    void free();

    void generate();
    void update();
    void render();
    void draw(Batch_Renderer *r);

    rnd_pcg_t make_rng_for_chunk();
};

struct World {
    siv::PerlinNoise noise;

    u32 seed;

    struct {
        ivec2 key;
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

    void set_projection(mat4 proj) {
        glProgramUniformMatrix4fv(chunk_shader, u_proj, 1, GL_FALSE, proj.value_ptr());
    }

    Chunk* get_chunk(s32 x, s32 y) {
        ivec2 key = {x, y};
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

    void update() {
        // TODO: So many things. So many.
        // First of all, well, yeah. Many things. Lots of things.
        // Every things.
        //
        // We should be keeping track of which chunks are "active"
        // somehow. This will tie in with the chunk loading/unloading
        // stuff deeply (probably). And of course we'll have "forced" chunks
        // which are _always_ active since they contain user-placed
        // tiles which must always be updated.
        // etc. etc. etc.
        //
        //                  - sci4me, 5/13/20

        for(u32 i = 0; i < hmlen(chunks); i++) {
            chunks[i].value->update();
        }
    }

    u32 draw_around(Batch_Renderer *r, vec2 pos, f32 scale, s32 window_width, s32 window_height, mat4 view) {
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
        glProgramUniformMatrix4fv(chunk_shader, u_view, 1, GL_FALSE, view.value_ptr());

        glUseProgram(chunk_shader);
        for(s32 i = vp_min_cx; i < vp_max_cx; i++) {
            for(s32 j = vp_min_cy; j < vp_max_cy; j++) {
                Chunk *c = get_chunk(i, j);
                c->draw(r);
            }
        }
        glUseProgram(0);

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

    for(u32 i = 0; i < hmlen(layer1); i++) {
        layer1[i].value->free();
        ::free(layer1[i].value);
    }
    for(u32 i = 0; i < hmlen(layer2); i++) {
        layer2[i].value->free();
        ::free(layer2[i].value);
    }
    hmfree(layer1);
    hmfree(layer2);
}

void Chunk::generate() {
    // TODO

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

    constexpr f32 cobblestone_frequency = 150.0f;
    constexpr f32 cobblestone_threshold = 0.20f;
    constexpr f32 coal_frequency = 100.0f;
    constexpr f32 coal_threshold = 0.25f;
    constexpr f32 iron_frequency = 80.0f;
    constexpr f32 iron_threshold = 0.20f;
    constexpr f32 gold_frequency = 50.0f;
    constexpr f32 gold_threshold = 0.2f;

    for(s32 i = 0; i < SIZE; i++) {
        for(s32 j = 0; j < SIZE; j++) {
            {
                f32 m = world->noise.noise2D_0_1(
                    ((f32) ((x * SIZE) + i)) / cobblestone_frequency, 
                    ((f32) ((y * SIZE) + j)) / cobblestone_frequency
                );

                if(m < coal_threshold) {
                    auto tile_mem = malloc(sizeof(Tile_Ore));
                    auto tile = new(tile_mem) Tile_Ore;
                    tile->type = TILE_COBBLESTONE;
                    tile->x = x * SIZE + i;
                    tile->y = y * SIZE + j;
                    tile->count = 100; // TODO
                    tile->initial_count = tile->count;
                    tile->init();

                    ivec2 key = {i, j};
                    hmput(layer1, key, tile);
                    
                    continue;
                }
            }

            {
                f32 m = world->noise.noise2D_0_1(
                    ((f32) ((x * SIZE) + i)) / coal_frequency, 
                    ((f32) ((y * SIZE) + j)) / coal_frequency
                );

                if(m < coal_threshold) {
                    auto tile_mem = malloc(sizeof(Tile_Ore));
                    auto tile = new(tile_mem) Tile_Ore;
                    tile->type = TILE_COAL_ORE;
                    tile->x = x * SIZE + i;
                    tile->y = y * SIZE + j;
                    tile->count = 100; // TODO
                    tile->initial_count = tile->count;
                    tile->init();

                    ivec2 key = {i, j};
                    hmput(layer1, key, tile);
                    
                    continue;
                }
            }

            {
                f32 m = world->noise.noise2D_0_1(
                    ((f32) ((x * SIZE) + i)) / iron_frequency, 
                    ((f32) ((y * SIZE) + j)) / iron_frequency
                );

                if(m < iron_threshold) {
                    auto tile_mem = malloc(sizeof(Tile_Ore));
                    auto tile = new(tile_mem) Tile_Ore;
                    tile->type = TILE_IRON_ORE;
                    tile->x = x * SIZE + i;
                    tile->y = y * SIZE + j;
                    tile->count = 100; // TODO
                    tile->initial_count = tile->count;
                    tile->init();

                    ivec2 key = {i, j};
                    hmput(layer1, key, tile);
                    
                    continue;
                }
            }

            {
                f32 m = world->noise.noise2D_0_1(
                    ((f32) ((x * SIZE) + i)) / gold_frequency, 
                    ((f32) ((y * SIZE) + j)) / gold_frequency
                );

                if(m < gold_threshold) {
                    auto tile_mem = malloc(sizeof(Tile_Ore));
                    auto tile = new(tile_mem) Tile_Ore;
                    tile->type = TILE_GOLD_ORE;
                    tile->x = x * SIZE + i;
                    tile->y = y * SIZE + j;
                    tile->count = 100; // TODO
                    tile->initial_count = tile->count;
                    tile->init();

                    ivec2 key = {i, j};
                    hmput(layer1, key, tile);
                    
                    continue;
                }
            }
        }
    }
}

Static_Array<Chunk::Vertex, Chunk::MAX_VERTICES> chunk_vertex_buffer;
Static_Array<u32, Chunk::MAX_INDICES> chunk_index_buffer;

// NOTE: This method is _not_ reentrant (or thread safe)!
void Chunk::render() {
    chunk_vertex_buffer.clear();
    chunk_index_buffer.clear();
    
    textures.clear();

    rnd_pcg_t l0rot = make_rng_for_chunk();

    static constexpr vec2 uvs[4][4] = {
        { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } },
        { { 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f } },
        { { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
        { { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f } }
    };

    for(s32 i = 0; i < SIZE; i++) {
        for(s32 j = 0; j < SIZE; j++) {
            auto tile = layer0[i][j];
            auto texture = tile_textures[tile].id;

            s32 tex_index = textures.alloc(texture);
            assert(tex_index != -1); // TODO

            f32 k = ((x * SIZE) + i) * TILE_SIZE;
            f32 l = ((y * SIZE) + j) * TILE_SIZE;

            auto rot = rnd_pcg_range(&l0rot, 0, 3);

            u32 tl = chunk_vertex_buffer.push({ {k, l}, uvs[rot][0], tex_index });
            u32 tr = chunk_vertex_buffer.push({ {k + TILE_SIZE, l}, uvs[rot][1], tex_index });
            u32 br = chunk_vertex_buffer.push({ {k + TILE_SIZE, l + TILE_SIZE}, uvs[rot][2], tex_index });
            u32 bl = chunk_vertex_buffer.push({ {k, l + TILE_SIZE}, uvs[rot][3], tex_index });

            chunk_index_buffer.push(tl);
            chunk_index_buffer.push(tr);
            chunk_index_buffer.push(br);
            chunk_index_buffer.push(br);
            chunk_index_buffer.push(bl);
            chunk_index_buffer.push(tl);
        }
    }

    assert(chunk_vertex_buffer.count == MAX_VERTICES);
    assert(chunk_index_buffer.count == MAX_INDICES);

    vbo.set_data(&chunk_vertex_buffer.data, MAX_VERTICES * sizeof(Vertex));
    ibo.set_data(&chunk_index_buffer.data, MAX_INDICES * sizeof(u32));
}

void Chunk::draw(Batch_Renderer *r) {
    vao.bind();

    for(u32 i = 0; i < textures.count; i++)
        glBindTextureUnit(i, textures.slots[i]);

    glDrawElements(GL_TRIANGLES, MAX_INDICES, GL_UNSIGNED_INT, 0);

    for(u32 i = 0; i < textures.count; i++)
        glBindTextureUnit(i, 0);

    vao.unbind();


    for(u32 k = 0; k < hmlen(layer1); k++) {
        auto p = layer1[k].key;
        auto tile = layer1[k].value;
        tile->draw(r);
    }

    // TODO: This is just a copy/paste of the above! FIX IT!
    // i.e. Don't duplicate the code lol
    // ... er... maybe? er... yeah.....
    for(u32 k = 0; k < hmlen(layer2); k++) {
        auto p = layer2[k].key;
        auto tile = layer2[k].value;
        tile->draw(r);
    }
}

void Chunk::update() {
    // NOTE: layer1 (and layer0, obviously) tiles don't get dynamic updates
    for(u32 i = 0; i < hmlen(layer2); i++) {
        auto tile = layer2[i].value;
        if(tile->flags & TILE_FLAG_WANTS_DYNAMIC_UPDATES) tile->update();
    }
}

rnd_pcg_t Chunk::make_rng_for_chunk() {
    rnd_pcg_t rand;
    rnd_pcg_seed(&rand, x * world->seed + y * world->seed);
    return rand;
}