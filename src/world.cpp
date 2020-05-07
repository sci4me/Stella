#define TILE_SIZE 16.0f

enum TileType {
    TILE_STONE = 0,
    TILE_GRASS = 1,

    N_TILE_TYPES
};

// TODO: Convert this to a texture atlas once
//       we figure out how to fix the artifacts...
Texture tile_textures[N_TILE_TYPES];

void load_tile_textures() {
    tile_textures[TILE_STONE] = load_texture_from_file("res/textures/stone.png");
    tile_textures[TILE_GRASS] = load_texture_from_file("res/textures/grass.png");
}

struct Chunk {
    static const s32 SIZE = 32; // must be a power of 2!

    struct World *world;
    s32 x;
    s32 y;
    TileType tiles[SIZE][SIZE];

    void generate();
};

struct World {
    siv::PerlinNoise noise;

    struct {
        glm::ivec2 key;
        struct Chunk *value;
    } *chunks = NULL;

    void init() {

    }

    void free() {
        for(u32 i = 0; i < hmlen(chunks); i++) {
            ::free(chunks[i].value);
        }
        hmfree(chunks);
    }

    Chunk* get_chunk(s32 x, s32 y) {
        glm::ivec2 key = {x, y};
        s64 i = hmgeti(chunks, key);

        if(i == -1) {
            Chunk *c = (Chunk*) calloc(1, sizeof(Chunk));
            c->world = this;
            c->x = x;
            c->y = y;
            c->generate();

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

    TileType world_get_tile(s32 x, s32 y) {
        Chunk *c = get_chunk_containing(x, y);
        return c->tiles[x & (Chunk::SIZE-1)][y & (Chunk::SIZE-1)];
    }

    void set_tile(s32 x, s32 y, TileType type) {
        assert(type != N_TILE_TYPES);
        Chunk *c = get_chunk_containing(x, y);
        c->tiles[x & (Chunk::SIZE-1)][y & (Chunk::SIZE-1)] = type;
    }

    void render_around_player(Batch_Renderer *r, glm::vec2 pos, f32 scale) {
        // TODO: Make this actually correct, lol.
        //       Currently it "works" well enough, but...
        //       there are artifacts, etc.

        f32 x = pos.x;
        f32 y = pos.y;

        s32 vp_min_x = (s32) floor((x - 640.0f / scale) / TILE_SIZE);
        s32 vp_min_y = (s32) floor((y - 360.0f / scale) / TILE_SIZE);
        s32 vp_max_x = (s32) ceil((x + 640.0f / scale) / TILE_SIZE);
        s32 vp_max_y = (s32) ceil((y + 360.0f / scale) / TILE_SIZE);

        s32 vp_min_cx = (s32) floor((f32)vp_min_x / (f32)Chunk::SIZE);
        s32 vp_min_cy = (s32) floor((f32)vp_min_y / (f32)Chunk::SIZE);
        s32 vp_max_cx = (s32) ceil((f32)vp_max_x / (f32)Chunk::SIZE);
        s32 vp_max_cy = (s32) ceil((f32)vp_max_y / (f32)Chunk::SIZE);

        for(s32 i = vp_min_cx; i < vp_max_cx; i++) {
            for(s32 j = vp_min_cy; j < vp_max_cy; j++) {
                Chunk *c = get_chunk(i, j);

                for(s32 k = 0; k < Chunk::SIZE; k++) {
                    for(s32 l = 0; l < Chunk::SIZE; l++) {
                        f32 tx = ((i * Chunk::SIZE) + k) * (f32)TILE_SIZE - x;
                        f32 ty = ((j * Chunk::SIZE) + l) * (f32)TILE_SIZE - y;

                        if(
                            tx + 2 * (TILE_SIZE * scale) < (-640.0f / scale) || 
                            ty + 2 * (TILE_SIZE * scale) < (-360.0f / scale) || 
                            tx - 2 * (TILE_SIZE * scale) > (640.0f / scale) || 
                            ty - 2 * (TILE_SIZE * scale) > (360.0f / scale)
                        ) continue;
            
                        r->push_textured_quad(tx, ty, TILE_SIZE, TILE_SIZE, &tile_textures[(u32)c->tiles[k][l]]);
                    }
                }
            }
        }
    }
};

void Chunk::generate() {
    constexpr f32 frequency = 70.0f;
    constexpr f32 stone_threshold = 0.3f;

    for(s32 i = 0; i < SIZE; i++) {
        for(s32 j = 0; j < SIZE; j++) {
            f32 m = world->noise.noise2D_0_1(
                ((f32) ((x * SIZE) + i)) / frequency, 
                ((f32) ((y * SIZE) + j)) / frequency
            );

            if(m < stone_threshold) tiles[i][j] = TILE_STONE;
            else                    tiles[i][j] = TILE_GRASS;
        }
    }
}