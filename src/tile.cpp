constexpr f32 TILE_SIZE = 32.0f;


enum Tile_Type : u8 {
    TILE_STONE,
    TILE_GRASS,

    TILE_COBBLESTONE,
    TILE_COAL_ORE,
    TILE_IRON_ORE,
    TILE_GOLD_ORE,

    TILE_CHEST,
    TILE_FURNACE,
    TILE_MINING_MACHINE,

    N_TILE_TYPES
};


Texture tile_textures[N_TILE_TYPES];

void init_tiles() {
    tile_textures[TILE_STONE] = assets::textures::stone;
    tile_textures[TILE_GRASS] = assets::textures::grass;
    tile_textures[TILE_COBBLESTONE] = assets::textures::cobblestone[array_length(assets::textures::coal_ore) - 1];
    tile_textures[TILE_COAL_ORE] = assets::textures::coal_ore[array_length(assets::textures::coal_ore) - 1];
    tile_textures[TILE_IRON_ORE] = assets::textures::iron_ore[array_length(assets::textures::iron_ore) - 1];
    tile_textures[TILE_GOLD_ORE] = assets::textures::gold_ore[array_length(assets::textures::gold_ore) - 1];
    tile_textures[TILE_CHEST] = assets::textures::chest;
    tile_textures[TILE_FURNACE] = assets::textures::furnace;
    tile_textures[TILE_MINING_MACHINE] = assets::textures::mining_machine;
}


typedef u8 Tile_Flags;
enum Tile_Flags_ : u8 {
    TILE_FLAG_NONE                          = 0,
    TILE_FLAG_WANTS_DYNAMIC_UPDATES         = 1,
    TILE_FLAG_IS_COLLIDER                   = 2
};


struct Tile {
    Tile_Type type;
    s32 x;
    s32 y;
    Tile_Flags flags;

    // NOTE: I don't _love_ having this here.
    // Maybe we just compute it as needed
    // if the flag is set? *shrugs*
    //          - sci4me, 5/15/20
    AABB collision_aabb;

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
            case TILE_COBBLESTONE:  textures = assets::textures::cobblestone; break;
            case TILE_COAL_ORE:     textures = assets::textures::coal_ore; break;
            case TILE_IRON_ORE:     textures = assets::textures::iron_ore; break;
            case TILE_GOLD_ORE:     textures = assets::textures::gold_ore; break;
            default:                assert(0); break;
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

        // NOTE TODO: We probably can just change this to use 1 texture!
        // Just change the UVs! "shrink" them, if you will. Whether it be
        // by using a hardcoded array of UVs or, more likely, just
        // calculating them.
        //                  - sci4me, 5/18/20

        r->push_textured_quad(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, texture);
    }
};

struct Tile_Chest : public Tile {
    Item_Container container;

    virtual void init() override {
        Tile::init();
        flags |= TILE_FLAG_IS_COLLIDER;

        // NOTE: See comment in Tile_Furnace::init about this.
        vec2 wp = { (f32)x * TILE_SIZE, (f32)y * TILE_SIZE };
        collision_aabb = {
            wp + vec2(4.0f, 4.0f),
            wp + vec2(28.0f, 28.0f)
        };

        container.init(25);
    }

    virtual void free() override {
        Tile::free();

        container.free();
    }
};


// NOTE TODO: Eventually we'll probably want to move the "smelting"
// code somewhere else, possibly "crafting.cpp". Not sure yet.
// This will become especially relevant once we have >1 kind of
// "smelting machine" (furnace).
// Or, for example, if we ever want to handle recipes in a way
// that is not hardcoded (i.e. load from file, etc.)
//                  - sci4me, 5/18/20
constexpr u32 COAL_FUEL_POINTS          = 800;
constexpr u32 FUEL_POINTS_PER_SMELT     = 100;

struct Smelting_Recipe {
    Item_Type input;
    Item_Type output;
};

const Smelting_Recipe smelting_recipes[] = {
    { ITEM_IRON_ORE, ITEM_IRON_PLATE },
    { ITEM_GOLD_ORE, ITEM_GOLD_PLATE }
};

struct Tile_Furnace : public Tile {
    Item_Container input;
    Item_Container fuel;
    Item_Container output;

    u32 fuel_points;
    bool is_smelting;
    Item_Type smelting_input_type;
    Item_Type smelting_output_type;
    u32 smelting_progress;

    virtual void init() override {
        Tile::init();
        flags |= TILE_FLAG_WANTS_DYNAMIC_UPDATES;
        flags |= TILE_FLAG_IS_COLLIDER;

        fuel_points = 0;
        is_smelting = false;
        smelting_progress = 0;

        // NOTE TODO: Don't hardcode these numbers!!
        // They're 4 and 28 because our tile size is 32;
        // if our tile size were the same as the pixel width
        // and height of our textures (16), this would be 2
        // and 14, in order to select the inner 12x12 pixels,
        // with a 2 pixel border on all sides.
        //              - sci4me, 5/15/20
        vec2 wp = { (f32)x * TILE_SIZE, (f32)y * TILE_SIZE };
        collision_aabb = {
            wp + vec2(4.0f, 4.0f),
            wp + vec2(28.0f, 28.0f)
        };

        input.init(1, ITEM_CONTAINER_FLAG_FILTER_INSERTIONS);
        
        for(u32 i = 0; i < array_length(smelting_recipes); i++) {
            auto& r = smelting_recipes[i];
            input.insertion_filter.set(r.input);
        }
        
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
        // TODO: Is this method really as simple as possible?
        // Press X to doubt.

        // TODO: Only consume an item from the input slot
        // if we have fuel!

        if(input.slots[0].count && !is_smelting) {
            input.slots[0].count--;
            is_smelting = true;
            
            smelting_input_type = input.slots[0].type;
            
            smelting_output_type = N_ITEM_TYPES;
            for(u32 i = 0; i < array_length(smelting_recipes); i++) { // PERF NOTE: linear search
                if(smelting_recipes[i].input == smelting_input_type) {
                    smelting_output_type = smelting_recipes[i].output;
                    break;
                }
            }
            assert(smelting_output_type != N_ITEM_TYPES);
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


struct Tile_Mining_Machine : public Tile {
    Item_Container fuel;
    Item_Container output;

    virtual void init() override {
        Tile::init();
        flags |= TILE_FLAG_WANTS_DYNAMIC_UPDATES;
        flags |= TILE_FLAG_IS_COLLIDER;

        // NOTE: See comment in Tile_Furnace::init about this.
        vec2 wp = { (f32)x * TILE_SIZE, (f32)y * TILE_SIZE };
        collision_aabb = {
            wp + vec2(4.0f, 4.0f),
            wp + vec2(28.0f, 28.0f)
        };

        fuel.init(1);
        output.init(9);
    }

    virtual void free() override {
        Tile::free();

        fuel.free();
        output.free();
    }

    virtual void update() override {
        // TODO
    }
};

template<typename T>
T* make_tile() {
    auto t = malloc(sizeof(T));
    return new(t) T;
}

Tile* make_tile(Tile_Type type) {
    Tile *result;

    switch(type) {
        case TILE_CHEST: result = make_tile<Tile_Chest>(); break;
        case TILE_FURNACE: result = make_tile<Tile_Furnace>(); break;
        case TILE_MINING_MACHINE: result = make_tile<Tile_Mining_Machine>(); break;
        default: assert(0); break;
    }
    result->type = type;

    return result;
}