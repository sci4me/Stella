#define TILE_SIZE 32.0f


enum Tile_Type : u8 {
    TILE_NONE,
    TILE_STONE,
    TILE_GRASS,

    TILE_COAL_ORE,
    TILE_IRON_ORE,

    TILE_CHEST,
    TILE_FURNACE,

    N_TILE_TYPES
};


Texture tile_textures[N_TILE_TYPES];

void load_tile_textures() {
    tile_textures[TILE_STONE] = assets::textures::stone;
    tile_textures[TILE_GRASS] = assets::textures::grass;
    tile_textures[TILE_COAL_ORE] = assets::textures::coal_ore[6];
    tile_textures[TILE_IRON_ORE] = assets::textures::iron_ore[6];
    tile_textures[TILE_CHEST] = assets::textures::chest;
    tile_textures[TILE_FURNACE] = assets::textures::furnace;
}


typedef u8 Tile_Flags;
enum Tile_Flags_ : u8 {
    TILE_FLAG_NONE                          = 0,
    TILE_FLAG_WANTS_DYNAMIC_UPDATES         = 1
};


struct Tile {
    Tile_Type type;
    s32 x;
    s32 y;
    Tile_Flags flags;

    virtual void init() {}
    virtual void free() {}

    virtual void draw(Batch_Renderer *r) {
        r->push_textured_quad(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, &tile_textures[(u32) type]);
    }

    virtual void update() {}
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
        Tile::free();

        container.free();
    }
};


// NOTE TODO: Eventually we won't want these to 
// be hardcoded into the furnace!
constexpr u32 COAL_FUEL_POINTS          = 800;
constexpr u32 FUEL_POINTS_PER_SMELT     = 100;

struct Tile_Furnace : public Tile {
    Item_Container input;
    Item_Container fuel;
    Item_Container output;

    u32 fuel_points = 0;
    bool is_smelting = false;
    Item_Type smelting_input_type;
    Item_Type smelting_output_type;
    u32 smelting_progress = 0;

    virtual void init() override {
        Tile::init();
        flags |= TILE_FLAG_WANTS_DYNAMIC_UPDATES;

        input.init(1, ITEM_CONTAINER_FLAG_FILTER_INSERTIONS);
        input.insertion_filter.set(ITEM_IRON_ORE);

        fuel.init(1, ITEM_CONTAINER_FLAG_FILTER_INSERTIONS);
        fuel.insertion_filter.set(ITEM_COAL_ORE);
        
        output.init(1, ITEM_CONTAINER_FLAG_NO_INSERT);
    }

    virtual void free() {
        Tile::free();

        input.free();
        fuel.free();
        output.free();
    }

    virtual void update() override {
        if(input.slots[0].count && !is_smelting) {
            input.slots[0].count--;
            is_smelting = true;
            
            smelting_input_type = input.slots[0].type;
            smelting_output_type = ITEM_IRON_INGOT; // TODO: Don't just use ITEM_IRON_INGOT here!
        }

        if(is_smelting && smelting_progress == FUEL_POINTS_PER_SMELT) {
            if(output.slots[0].count) {
                if(output.slots[0].type == smelting_output_type && output.slots[0].count < MAX_ITEM_SLOT_SIZE) {
                    output.slots[0].count++;
                } else {
                    return;
                }
            } else {
                output.slots[0].type = smelting_output_type;
                output.slots[0].count = 1;
            }

            is_smelting = false;
            smelting_progress = 0;
            return;
        }

        if(is_smelting) {
            if(!fuel_points) {
                // NOTE: Don't have to check type since we have
                // the insertion filter.
                if(fuel.slots[0].count) {
                    fuel.slots[0].count--;
                    fuel_points += COAL_FUEL_POINTS;
                }
            }

            if(!fuel_points) return;

            fuel_points--;
            smelting_progress++;
        }
    }
};