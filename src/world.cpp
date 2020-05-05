#define TILE_SIZE 16.0f
#define CHUNK_SIZE 16 // must be a power of 2!

enum TileType {
    TILE_STONE,
    TILE_GRASS,

    N_TILE_TYPES
};

struct World {
    siv::PerlinNoise noise;

    struct {
        glm::ivec2 key;
        struct Chunk *value;
    } *chunks = NULL;
};

struct Chunk {
    World *world;
    s32 x;
    s32 y;
    TileType tiles[CHUNK_SIZE][CHUNK_SIZE];
};

void generate_chunk(Chunk *c) {
    constexpr f32 frequency = 10.0f;
    constexpr f32 stone_threshold = 0.5f;

    for(s32 i = 0; i < CHUNK_SIZE; i++) {
        for(s32 j = 0; j < CHUNK_SIZE; j++) {
            f32 m = c->world->noise.noise2D_0_1(
                ((f32) ((c->x * CHUNK_SIZE) + i)) / frequency, 
                ((f32) ((c->y * CHUNK_SIZE) + j)) / frequency
            );

            if(m < 0.5f) c->tiles[i][j] = TILE_STONE;
            else         c->tiles[i][j] = TILE_GRASS;
        }
    }
}

void world_init(World *w) {

}

void world_free(World *w) {
    for(u32 i = 0; i < hmlen(w->chunks); i++) {
        free(w->chunks[i].value);
    }
    hmfree(w->chunks);
}

Chunk* world_get_chunk(World *w, s32 x, s32 y) {
    glm::ivec2 key = {x, y};
    s64 i = hmgeti(w->chunks, key);

    if(i == -1) {
        Chunk *c = (Chunk*) calloc(1, sizeof(Chunk));
        c->world = w;
        c->x = x;
        c->y = y;
        generate_chunk(c);
        hmput(w->chunks, key, c);
        
        return c;
    }   

    return w->chunks[i].value;
}

Chunk* world_get_chunk_containing(World *w, s32 x, s32 y) {
    s32 cx = (s32)floor(x / (f32)CHUNK_SIZE);
    s32 cy = (s32)floor(y / (f32)CHUNK_SIZE);
    return world_get_chunk(w, cx, cy);
}

TileType world_get_tile(World *w, s32 x, s32 y) {
    Chunk *c = world_get_chunk_containing(w, x, y);
    return c->tiles[x & CHUNK_SIZE-1][y & CHUNK_SIZE-1];
}

void world_set_tile(World *w, s32 x, s32 y, TileType type) {
    assert(type != N_TILE_TYPES);
    Chunk *c = world_get_chunk_containing(w, x, y);
    c->tiles[x & CHUNK_SIZE-1][y & CHUNK_SIZE-1] = type;
}