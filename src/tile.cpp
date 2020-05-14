#define TILE_SIZE 32.0f


enum Tile_Type : u8 {
    TILE_NONE,
    TILE_STONE,
    TILE_GRASS,

    TILE_COAL_ORE,
    TILE_IRON_ORE,

    TILE_CHEST,

    N_TILE_TYPES
};


Texture tile_textures[N_TILE_TYPES];

void load_tile_textures() {
    tile_textures[TILE_STONE] = assets::textures::stone;
    tile_textures[TILE_GRASS] = assets::textures::grass;
    tile_textures[TILE_COAL_ORE] = assets::textures::coal_ore[6];
    tile_textures[TILE_IRON_ORE] = assets::textures::iron_ore[6];
    tile_textures[TILE_CHEST] = assets::textures::chest;
}


struct Tile {
    Tile_Type type;
    s32 x;
    s32 y;

    virtual void init() {}
    virtual void free() {}

    virtual void draw(Batch_Renderer *r) {
        r->push_textured_quad(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, &tile_textures[(u32) type]);
    }
};


struct Tile_Ore : public Tile {
    u32 count;
    u32 initial_count;
    Texture *textures;

    virtual void init() override {
        Tile::init();

        switch(type) {
            case TILE_COAL_ORE: textures = assets::textures::coal_ore; break;
            case TILE_IRON_ORE: textures = assets::textures::iron_ore; break;
            default: assert(0); break;
        }
    }

    virtual void draw(Batch_Renderer *r) override {
        Texture *texture;

        f32 p = (f32)count / (f32)initial_count;
        if(p < 0.1f) {
            texture = &textures[0];
        } else if(p >= 0.1f && p < 0.2f) {
            texture = &textures[1];
        } else if(p >= 0.2f && p < 0.3f) {
            texture = &textures[2];
        } else if(p >= 0.3f && p < 0.5f) {
            texture = &textures[3];
        } else if(p >= 0.5f && p < 0.7f) {
            texture = &textures[4];
        } else if(p >= 0.7f && p < 0.9f) {
            texture = &textures[5];
        } else {
            texture = &textures[6];
        }

        r->push_textured_quad(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, texture);
    }
};

struct Tile_Chest : public Tile {
    Item_Container container;

    virtual void init() override {
        Tile::init();

        container.init(25);
    }

    virtual void free() override {
        container.free();
    }
};