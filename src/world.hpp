#ifndef WORLD_H
#define WORLD_H

struct Chunk {
    #pragma pack(push, 1)
    struct Vertex {
        vec2 pos;
        vec2 uv;
        s32 tex;
    };
    #pragma pack(pop)

    static constexpr s32 SIZE = 64; // must be a power of 2!
    static constexpr s32 LAYERS = 3;

    static constexpr u32 MAX_VERTICES = SIZE * SIZE * 4;
    static constexpr u32 MAX_INDICES = SIZE * SIZE * 6;
    static constexpr u32 MAX_TEXTURE_SLOTS = 16; // TODO

    struct World *world;
    
    s32 x;
    s32 y;

    Tile_Type layer0[SIZE][SIZE];
    Hash_Table<ivec2, Tile*> layer1;
    Hash_Table<ivec2, Tile*> layer2;

    Vertex_Array vao;
    Vertex_Buffer vbo;
    Index_Buffer ibo;
    Slot_Allocator<GLuint, MAX_TEXTURE_SLOTS> textures;

    void init(struct World *world, s32 x, s32 y);
    void deinit();

    void generate();
    void update();
    void render();
    void draw(Batch_Renderer *r);

    rnd_pcg_t make_rng_for_chunk();
};

struct World {
    PerlinNoise noise;

    u32 seed;

    Hash_Table<ivec2, Chunk*> chunks;

    GLuint chunk_shader;
    GLuint u_textures;
    GLuint u_proj;
    GLuint u_view;

    void init(u32 seed = 1);
    void deinit();
    void set_projection(mat4 proj);
    Chunk* get_chunk(s32 x, s32 y);
    Chunk* get_chunk_containing(s32 x, s32 y);
    void update();
    u32 draw_around(Batch_Renderer *r, vec2 pos, f32 scale, s32 window_width, s32 window_height, mat4 view);

    Tile* get_tile_at(s32 x, s32 y, s32 layer);
    bool remove_tile_at(s32 x, s32 y, s32 layer);
};

#endif