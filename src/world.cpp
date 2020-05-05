#define TILE_SIZE 16.0f
#define CHUNK_SIZE 32

enum TileType {
    TILE_STONE,
    TILE_GRASS,

    N_TILES
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

TileType world_get_tile(World *w, s32 x, s32 y) {
    Chunk *c = world_get_chunk(w, x / CHUNK_SIZE, y / CHUNK_SIZE);
    return c->tiles[((s32) abs(x)) % CHUNK_SIZE][((s32) abs(y)) % CHUNK_SIZE];
}