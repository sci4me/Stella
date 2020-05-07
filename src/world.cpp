#define TILE_SIZE 16.0f
#define CHUNK_SIZE 32 // must be a power of 2!

enum TileType {
    TILE_STONE = 0,
    TILE_GRASS = 1,

    N_TILE_TYPES
};

// TODO: Convert this to a texture atlas once
//       we figure out how to fix the artifacts...
GLuint tile_textures[N_TILE_TYPES];

void load_tile_textures() {
    tile_textures[TILE_STONE] = load_texture("res/textures/stone.png");
    tile_textures[TILE_GRASS] = load_texture("res/textures/grass.png");
}

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
    constexpr f32 frequency = 70.0f;
    constexpr f32 stone_threshold = 0.3f;

    for(s32 i = 0; i < CHUNK_SIZE; i++) {
        for(s32 j = 0; j < CHUNK_SIZE; j++) {
            f32 m = c->world->noise.noise2D_0_1(
                ((f32) ((c->x * CHUNK_SIZE) + i)) / frequency, 
                ((f32) ((c->y * CHUNK_SIZE) + j)) / frequency
            );

            if(m < stone_threshold) c->tiles[i][j] = TILE_STONE;
            else                    c->tiles[i][j] = TILE_GRASS;
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
    return c->tiles[x & (CHUNK_SIZE-1)][y & (CHUNK_SIZE-1)];
}

void world_set_tile(World *w, s32 x, s32 y, TileType type) {
    assert(type != N_TILE_TYPES);
    Chunk *c = world_get_chunk_containing(w, x, y);
    c->tiles[x & (CHUNK_SIZE-1)][y & (CHUNK_SIZE-1)] = type;
}

void world_render_around_player(World *w, Batch_Renderer *r, glm::vec2 pos, f32 scale) {
    // TODO: Make this actually correct, lol.
    //       Currently it "works" well enough, but...
    //       there are artifacts, etc.

    f32 x = pos.x;
    f32 y = pos.y;

    s32 vp_min_x = (s32) floor((x - 640.0f / scale) / TILE_SIZE);
    s32 vp_min_y = (s32) floor((y - 360.0f / scale) / TILE_SIZE);
    s32 vp_max_x = (s32) ceil((x + 640.0f / scale) / TILE_SIZE);
    s32 vp_max_y = (s32) ceil((y + 360.0f / scale) / TILE_SIZE);

    s32 vp_min_cx = (s32) floor((f32)vp_min_x / (f32)CHUNK_SIZE);
    s32 vp_min_cy = (s32) floor((f32)vp_min_y / (f32)CHUNK_SIZE);
    s32 vp_max_cx = (s32) ceil((f32)vp_max_x / (f32)CHUNK_SIZE);
    s32 vp_max_cy = (s32) ceil((f32)vp_max_y / (f32)CHUNK_SIZE);

    for(s32 i = vp_min_cx; i < vp_max_cx; i++) {
        for(s32 j = vp_min_cy; j < vp_max_cy; j++) {
            Chunk *c = world_get_chunk(w, i, j);

            for(s32 k = 0; k < CHUNK_SIZE; k++) {
                for(s32 l = 0; l < CHUNK_SIZE; l++) {
                    f32 tx = ((i * CHUNK_SIZE) + k) * (f32)TILE_SIZE - x;
                    f32 ty = ((j * CHUNK_SIZE) + l) * (f32)TILE_SIZE - y;

                    if(
                        tx + 2 * (TILE_SIZE * scale) < (-640.0f / scale) || 
                        ty + 2 * (TILE_SIZE * scale) < (-360.0f / scale) || 
                        tx - 2 * (TILE_SIZE * scale) > (640.0f / scale) || 
                        ty - 2 * (TILE_SIZE * scale) > (360.0f / scale)
                    ) continue;
        
                    r->push_textured_quad(tx, ty, TILE_SIZE, TILE_SIZE, tile_textures[(u32)c->tiles[k][l]]);
                }
            }
        }
    }
}