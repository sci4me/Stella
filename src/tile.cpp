#define TILE_SIZE 32.0f


enum Tile_Type : u8 {
    TILE_NONE,
    TILE_STONE,
    TILE_GRASS,

    TILE_COAL_ORE,

    TILE_CHEST,

    N_TILE_TYPES
};


// TODO: Convert this to a texture atlas once
//       we figure out how to fix the artifacts...
Texture tile_textures[N_TILE_TYPES];

// TODO: array_length macro? i.e. #define array_length(a) (sizeof(a)/sizeof(typeof(a[0])))
constexpr u32 N_COAL_TEXTURES = 7;
Texture coal_textures[N_COAL_TEXTURES];

void load_tile_textures() {
    constexpr bool MIPS = true;

    tile_textures[TILE_STONE]       = load_texture_from_file("res/textures/tile/stone.png", MIPS);
    tile_textures[TILE_GRASS]       = load_texture_from_file("res/textures/tile/grass.png", MIPS);
    tile_textures[TILE_COAL_ORE]    = load_texture_from_file("res/textures/tile/coal_ore_100.png", MIPS);
    tile_textures[TILE_CHEST]       = load_texture_from_file("res/textures/tile/chest.png", MIPS);

    coal_textures[6]                = tile_textures[TILE_COAL_ORE];
    coal_textures[5]                = load_texture_from_file("res/textures/tile/coal_ore_90.png", MIPS);
    coal_textures[4]                = load_texture_from_file("res/textures/tile/coal_ore_70.png", MIPS);
    coal_textures[3]                = load_texture_from_file("res/textures/tile/coal_ore_50.png", MIPS);
    coal_textures[2]                = load_texture_from_file("res/textures/tile/coal_ore_30.png", MIPS);
    coal_textures[1]                = load_texture_from_file("res/textures/tile/coal_ore_20.png", MIPS);
    coal_textures[0]                = load_texture_from_file("res/textures/tile/coal_ore_10.png", MIPS);
}


struct Tile {
    Tile_Type type;
    s32 x;
    s32 y;

    virtual void draw(Batch_Renderer *r) {
        r->push_textured_quad(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, &tile_textures[(u32) type]);
    }
};


struct Tile_Ore : public Tile {
    u32 count;
    u32 initial_count;

    virtual void draw(Batch_Renderer *r) override {
        Texture *texture;

        f32 p = (f32)count / (f32)initial_count;
        if(p < 0.1f) {
            texture = &coal_textures[0];
        } else if(p >= 0.1f && p < 0.2f) {
            texture = &coal_textures[1];
        } else if(p >= 0.2f && p < 0.3f) {
            texture = &coal_textures[2];
        } else if(p >= 0.3f && p < 0.5f) {
            texture = &coal_textures[3];
        } else if(p >= 0.5f && p < 0.7f) {
            texture = &coal_textures[4];
        } else if(p >= 0.7f && p < 0.9f) {
            texture = &coal_textures[5];
        } else {
            texture = &coal_textures[6];
        }

        r->push_textured_quad(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, texture);
    }
};

struct Tile_Chest : public Tile {
    // TODO
};