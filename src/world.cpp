#define TILE_SIZE 16.0f

enum TileType {
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
    static const s32 SIZE = 32; // must be a power of 2!
    static const s32 LAYERS = 3;

    struct World *world;
    s32 x;
    s32 y;
    TileType tiles[SIZE][SIZE][LAYERS];

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

    TileType get_tile(s32 x, s32 y, s32 layer) {
        Chunk *c = get_chunk_containing(x, y);
        return c->tiles[x & (Chunk::SIZE-1)][y & (Chunk::SIZE-1)][layer];
    }

    void set_tile(s32 x, s32 y, s32 layer, TileType type) {
        assert(type != N_TILE_TYPES);
        Chunk *c = get_chunk_containing(x, y);
        c->tiles[x & (Chunk::SIZE-1)][y & (Chunk::SIZE-1)][layer] = type;
    }

    void render_around(Batch_Renderer *r, glm::vec2 pos, f32 scale, s32 window_width, s32 window_height) {
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

        /*
        ImGui::Begin("render_around", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::Dummy(ImVec2(110, 0));
        ImGui::Text("vp_min: (%d, %d)", vp_min_x, vp_min_y);
        ImGui::Text("vp_max: (%d, %d)", vp_max_x, vp_max_y);
        ImGui::Text("delta: (%d, %d)", vp_max_x - vp_min_x, vp_max_y - vp_min_y);
        ImGui::Separator();
        ImGui::Text("vp_min_c: (%d, %d)", vp_min_cx, vp_min_cy);
        ImGui::Text("vp_max_c: (%d, %d)", vp_max_cx, vp_max_cy);
        ImGui::Text("delta_c: (%d, %d)", vp_max_cx - vp_min_cx, vp_max_cy - vp_min_cy);
        ImGui::End();
        */

        for(s32 i = vp_min_cx; i < vp_max_cx; i++) {
            for(s32 j = vp_min_cy; j < vp_max_cy; j++) {
                Chunk *c = get_chunk(i, j);

                for(s32 k = 0; k < Chunk::SIZE; k++) {
                    for(s32 l = 0; l < Chunk::SIZE; l++) {
                        s32 m = (i * Chunk::SIZE) + k;
                        s32 n = (j * Chunk::SIZE) + l;

                        if(m < vp_min_x || n < vp_min_y || m > vp_max_x || n > vp_max_y) continue;

                        for(s32 layer = 0; layer < Chunk::LAYERS; layer++) {
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
};

void Chunk::generate() {
    constexpr f32 stone_frequency = 70.0f;
    constexpr f32 stone_threshold = 0.3f;

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
    constexpr f32 coal_threshold = 0.2f;

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