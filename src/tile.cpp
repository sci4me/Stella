// TODO(URGENT): FIX THIS! Hot code reloading only
// makes this WAY worse...
Texture tile_textures[N_TILE_TYPES];

void init_tiles(Assets *assets) {
    tile_textures[TILE_STONE]               = assets->tile_textures[TILE_STONE];
    tile_textures[TILE_GRASS]               = assets->tile_textures[TILE_GRASS];
    tile_textures[TILE_COBBLESTONE]         = assets->tile_textures[TILE_COBBLESTONE];
    tile_textures[TILE_COAL_ORE]            = assets->tile_textures[TILE_COAL_ORE];
    tile_textures[TILE_IRON_ORE]            = assets->tile_textures[TILE_IRON_ORE];
    tile_textures[TILE_GOLD_ORE]            = assets->tile_textures[TILE_GOLD_ORE];
    tile_textures[TILE_CHEST]               = assets->tile_textures[TILE_CHEST];
    tile_textures[TILE_FURNACE]             = assets->tile_textures[TILE_FURNACE];
    tile_textures[TILE_MINING_MACHINE]      = assets->tile_textures[TILE_MINING_MACHINE];
}


typedef u8 Tile_Flags;
enum Tile_Flags_ : u8 {
    TILE_FLAG_NONE                          = 0,
    TILE_FLAG_WANTS_DYNAMIC_UPDATES         = 1,
    TILE_FLAG_IS_COLLIDER                   = 2
};


struct Tile {
    struct World *world;
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
    virtual void deinit() {}

    virtual void draw(Batch_Renderer *r) {
        TIMED_FUNCTION();

        r->push_textured_quad(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, &tile_textures[(u32) type]);
    }

    virtual void update() {}
};


struct Tile_Ore : public Tile {
    u32 count;
    u32 initial_count;
    u32 rot;

    virtual void init() override {
        Tile::init();
    }

    virtual void draw(Batch_Renderer *r) override {
        TIMED_FUNCTION();

        f32 p = (f32)count / (f32)initial_count;
        f32 invp = 1.0f - p;

        vec2 uvs[4];
        mlc_memcpy(&uvs, QUAD_UVS[rot], sizeof(vec2) * 4);

        f32 h = invp * 0.5f;
        for(u32 i = 0; i < array_length(uvs); i++) {
            s32 us = uvs[i].x == 1.0f ? -1 : 1;
            s32 vs = uvs[i].y == 1.0f ? -1 : 1;
            uvs[i] += vec2(h * us, h * vs);
        }

        f32 H = invp * TILE_SIZE * 0.5f;
        f32 H2 = invp * TILE_SIZE;
        r->push_quad(
            x * TILE_SIZE + H,
            y * TILE_SIZE + H,
            TILE_SIZE - H2,
            TILE_SIZE - H2,
            vec4(1.0f, 1.0f, 1.0f, 1.0f),
            uvs,
            tile_textures[(u32) type].id
        );
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

    virtual void deinit() override {
        Tile::deinit();

        container.deinit();
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

    virtual void deinit() {
        Tile::deinit();

        input.deinit();
        fuel.deinit();
        output.deinit();
    }

    virtual void update() override {
        TIMED_FUNCTION();

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

    u32 fuel_points;

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

        fuel.init(1, ITEM_CONTAINER_FLAG_FILTER_INSERTIONS);
        fuel.insertion_filter.set(ITEM_COAL_ORE);

        output.init(1, ITEM_CONTAINER_FLAG_NO_INSERT);
    }

    virtual void deinit() override {
        Tile::deinit();

        fuel.deinit();
        output.deinit();
    }

    virtual void update() override {
        TIMED_FUNCTION();

        if(fuel.slots[0].count) {
            fuel.slots[0].count--;
            fuel_points += COAL_FUEL_POINTS;
        }

        if(fuel_points > 0) {
            // auto chunk = world->get_chunk_containing(x, y);
            // auto idx = chunk->layer1.index_of(ivec2(x & (Chunk::SIZE - 1), y & (Chunk::SIZE - 1)));
            // if(idx == -1) return;
        }
    }
};


// NOTE: The following crap is just to make sure
// that the vtables get initialized, which happens
// sometime during the ctor...
// Long-term, we should probably just remove all
// use of virtual methods.

template<typename T>
T* make_tile() {
    void *t = mlc_malloc(sizeof(T));
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