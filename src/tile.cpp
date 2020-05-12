#define TILE_SIZE 32.0f


enum Tile_Type : u8 {
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


struct Tile {
    Tile_Type type;
};


struct Tile_Ore : public Tile {
    u32 count;
};  