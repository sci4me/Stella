void Tile::init() {}
void Tile::deinit() {}
void Tile::draw(Batch_Renderer *r) {
    TIMED_FUNCTION();

    r->push_textured_quad(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, &g_inst->assets->tile_textures[(u32) type]);
}
void Tile::update() {}
void Tile::get_bounding_boxes(Dynamic_Array<AABB> *bbs) {}


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
            g_inst->assets->tile_textures[(u32) type].id
        );
    }
};


struct Tile_Chest : public Tile {
    Item_Container container;

    virtual void init() override {
        Tile::init();
        flags |= TILE_FLAG_IS_COLLIDER;

        container.init(25);
    }

    virtual void deinit() override {
        Tile::deinit();

        container.deinit();
    }

    virtual void get_bounding_boxes(Dynamic_Array<AABB> *bbs) override {
        bbs->push({
            vec2(4.0f, 4.0f),
            vec2(28.0f, 28.0f)
        });
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

constexpr Smelting_Recipe smelting_recipes[] = {
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

    virtual void get_bounding_boxes(Dynamic_Array<AABB> *bbs) override {
        // NOTE TODO: Don't hardcode these numbers!!
        // They're 4 and 28 because our tile size is 32;
        // if our tile size were the same as the pixel width
        // and height of our textures (16), this would be 2
        // and 14, in order to select the inner 12x12 pixels,
        // with a 2 pixel border on all sides.
        //              - sci4me, 5/15/20
        bbs->push({
            vec2(4.0f, 4.0f),
            vec2(28.0f, 28.0f)
        });
    }
};


struct Tile_Mining_Machine : public Tile {
    Item_Container fuel;
    Item_Container output;

    u32 fuel_points;
    bool is_mining;
    u32 mining_progress;

    virtual void init() override {
        Tile::init();
        flags |= TILE_FLAG_WANTS_DYNAMIC_UPDATES;
        flags |= TILE_FLAG_IS_COLLIDER;

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

        Tile *tile = world->get_tile_at(x, y, 1);

        if(tile) {
            is_mining = true;
        } else {
            is_mining = true;
            return;
        }

        Item_Type ore_type;
        switch(tile->type) {
            case TILE_COAL_ORE:
                ore_type = ITEM_COAL_ORE;
                break;
            case TILE_IRON_ORE:
                ore_type = ITEM_IRON_ORE;
                break;
            case TILE_GOLD_ORE:
                ore_type = ITEM_GOLD_ORE;
                break;
            default:
                is_mining = false;
                return;
        }
        Tile_Ore *ore = (Tile_Ore*) tile;
        assert(ore->count);

        // TODO: Replace that `100`
        // TODO: Simplify these if/else cases; make a method on
        // Item_Container if we can't just use a pre-existing one.
        if(is_mining && mining_progress == 100) {
            if(output.slots[0].count) {
                if(output.slots[0].type == ore_type && output.slots[0].count < MAX_ITEM_SLOT_SIZE) {
                    output.slots[0].count++;
                } else {
                    return;
                }
            } else {
                output.slots[0].type = ore_type;
                output.slots[0].count = 1;
            }

            ore->count--;
            if(ore->count == 0) {
                assert(world->remove_tile_at(x, y, 1));
            }

            is_mining = false;
            mining_progress = 0;
            return;
        }

        if(is_mining) {
            if(!fuel_points) {
                if(fuel.slots[0].count) {
                    fuel.slots[0].count--;
                    fuel_points += COAL_FUEL_POINTS;
                }
            }

            if(!fuel_points) return;

            fuel_points--;
            mining_progress++;
        }
    }

    virtual void get_bounding_boxes(Dynamic_Array<AABB> *bbs) override {
        // NOTE: See comment in Tile_Furnace::init about this.
        bbs->push({
            vec2(2.0f, 2.0f),
            vec2(30.0f, 30.0f)
        });
    }
};


struct Tile_Tube : public Tile {
    Direction connected_directions;

    Static_Bitset<4> connection_filter;

    virtual void init() override {
        Tile::init();
        flags |= TILE_FLAG_WANTS_DYNAMIC_UPDATES;
        flags |= TILE_FLAG_IS_COLLIDER;

        connection_filter.set(TILE_CHEST);
        connection_filter.set(TILE_FURNACE);
        connection_filter.set(TILE_MINING_MACHINE);
        connection_filter.set(TILE_TUBE);
    }

    virtual void deinit() override {
        Tile::deinit();
    }

    virtual void draw(Batch_Renderer *r) override {
        Tile::draw(r);

        #define _X(id, value, dx, dy, ord, opp) draw_connection(r, id, ord);
        DIRECTIONS(_X)
        #undef _X
    }

    virtual void update() override {
        connected_directions = 0;
        #define _X(id, value, dx, dy, ord, opp) if(Tile *t = world->get_tile_at(x + dx, y + dy, 2); t && connection_filter.get(t->type)) connected_directions |= value;
        DIRECTIONS(_X)
        #undef _X
    }

    virtual void get_bounding_boxes(Dynamic_Array<AABB> *bbs) override {
        // NOTE: See comment in Tile_Furnace::init about this.
        bbs->push({
            vec2(10.0f, 10.0f),
            vec2(22.0f, 22.0f)
        });
    
        if(connected_directions & DIR_NORTH) {
            bbs->push({
                vec2(10.0f, 0.0f),
                vec2(22.0f, 10.0f),
            });
        }

        if(connected_directions & DIR_SOUTH) {
            bbs->push({
                vec2(10.0f, 22.0f),
                vec2(22.0f, 32.0f),
            });
        }

        if(connected_directions & DIR_EAST) {
            bbs->push({
                vec2(22.0f, 10.0f),
                vec2(32.0f, 22.0f),
            });
        }

        if(connected_directions & DIR_WEST) {
            bbs->push({
                vec2(0.0f, 10.0f),
                vec2(10.0f, 22.0f),
            });
        }
    }

private:
    static constexpr u32 tube_tex[] = { TEX_TUBE_NORTH, TEX_TUBE_SOUTH, TEX_TUBE_EAST, TEX_TUBE_WEST };

    //
    // NOTE TODO: Putting this here because idk where to put it or whatever; could be in TODO but I want this to be
    // something that changes very soon so, meh.
    //
    // The reason this doesn't work is because Direction is not a traditional enum, it is flags for a 'bitfield'.
    // So, it goes 1, 2, 4, 8, rather than 0, 1, 2, 3. This seemed like a good idea but has proven to be more
    // problematic than it's worth.
    // We _do_ want the ability to have a bitfield for directions, perhaps... but it seems that we should make
    // this a separate thing from Direction itself.
    //
    // Regardless of that (^), this hacky solution will not actually work even as a temporary solution.
    // It will not work when a tube is connected to a mining machine since the mining machine has transparency
    // where the tube would be visible beneath, when it obviously shouldn't be.
    //
    // So, we have to do something smarter no matter what.
    //

    void draw_connection(Batch_Renderer *r, Direction dir, s32 ord) {
        if(connected_directions & dir) {
            r->push_textured_quad(
                x * TILE_SIZE, 
                y * TILE_SIZE, 
                TILE_SIZE, 
                TILE_SIZE, 
                &g_inst->assets->ancillary_textures[tube_tex[ord]]
            );

            //
            // NOTE: Okay, here's our first pass at solving this problem. Just gonna do the stupid simple thing.
            // We'll use the AABB of the other tile to figure out what region of the tube piece needs to be rendered.
            // Definltely hacky as hell! But for now it should work. UNTIL we want tubes to connect to _other_ tiles
            // that have >1 AABB.
            //

            s32 tx = x + DIR_X_OFFSET[ord];
            s32 ty = y + DIR_Y_OFFSET[ord];
            Tile *t = world->get_tile_at(tx, ty, 2);
            assert(t);

            if(t->type == TILE_TUBE) return;

            Dynamic_Array<AABB> bbs;
            bbs.init();

            t->get_bounding_boxes(&bbs);
            assert(bbs.count == 1);

            AABB const& bb = bbs[0];

            AABB draw_box = { vec2(0.0f, 0.0f), vec2(TILE_SIZE, TILE_SIZE) };
            vec2 uvs[] = {
                vec2(0.0f, 0.0f),
                vec2(1.0f, 0.0f),
                vec2(1.0f, 1.0f),
                vec2(0.0f, 1.0f)
            };

            Direction opp = DIR_OPPOSITE[ord];
            switch(opp) {
                case DIR_NORTH: {
                    draw_box.max.y = bb.min.y;
                    for(s32 i = 0; i <  array_length(uvs); i++) uvs[i].y *= (f32)draw_box.max.y / TILE_SIZE;
                    break;
                }
                case DIR_SOUTH: {
                    draw_box.min.y = bb.max.y;
                    draw_box.max.y = TILE_SIZE - bb.max.y;
                    for(s32 i = 0; i <  array_length(uvs); i++) {
                        f32 s = (f32)draw_box.max.y / TILE_SIZE;
                        uvs[i].y *= s;
                        uvs[i].y += 1.0f - s;
                    }
                    break;
                }
                case DIR_EAST: {
                    draw_box.min.x = bb.max.x;
                    draw_box.max.x = TILE_SIZE - bb.max.x;
                    for(s32 i = 0; i <  array_length(uvs); i++) {
                        f32 s = (f32)draw_box.max.x / TILE_SIZE;
                        uvs[i].x *= s;
                        uvs[i].x += 1.0f - s;
                    }
                    break;
                }
                case DIR_WEST: {
                    draw_box.max.x = bb.min.x;
                    for(s32 i = 0; i <  array_length(uvs); i++) uvs[i].x *= (f32)draw_box.max.x / TILE_SIZE;
                    break;
                }
                default: {
                    assert(0);
                }
            }

            s32 opp_ord = dir_ordinal(opp);
            assert(opp_ord != -1);

            r->push_quad(
                tx * TILE_SIZE + draw_box.min.x, 
                ty * TILE_SIZE + draw_box.min.y, 
                draw_box.max.x, 
                draw_box.max.y, 
                vec4(1.0f, 1.0f, 1.0f, 1.0f), 
                uvs, 
                g_inst->assets->ancillary_textures[tube_tex[opp_ord]].id
            );
        }
    }
};


//
//
// NOTE: So I was just working on trying to implement Tile_Mining_Machine::update,
// and I had a realization. The only things that a Tile will ever want to query
// from a World is other Tiles and Entities. So, that's interesting to consider...
//
// It sort of relates to the idea of figuring out how exactly we want to manage
// the memory for Tiles and Entitites. (Maybe they'll end up being just Entities? *shrugs*)
// 
// What we do now is just store Tiles in a hash table in the Chunks. This is fine for now 
// but could definitely be much better. Entities aren't a thing yet but they'd be sort of
// similar except we can't just store them in a hash table. The first implementation would
// probably just be storing them in an array and doing linear search to look them up.
// Although we may (mayyybe.... but.. maybe not really..?) avoid this by doing a slightly
// smarted allocation scheme using a generation count per Entity storage slot (we will do this
// either way actually) but, there's sure to still be an O(n) operation in there somewhere,
// other than for things like update and render. For example, allocation and deletion.
//
// It's all unclear rn lol.
//
// But the point was that this may mean that we don't necessarily have to give a Tile access to
// a World directly. It may be the case that it's better to have a sort of proxy interface for
// Tiles (and Entities...) to use to interact with the World.
//
//                  - sci4me, 6/3/20
//
// Just going to say a little more here...
// If/when we do multi-thread world updating, the model we're using right now breaks completely.
// For example, say one tile update function is running in thread 1 and it wants to query a tile
// in the world and do something with it; then, some other tile (or something else perhaps) destroys
// that tile (the one being queried in thread 1) _from thread 2_ between when thread 1 checks and uses
// the tile. https://en.wikipedia.org/wiki/Time-of-check_to_time-of-use
//
// So. I have no freaking idea what to do here rn lol. Not gonna worry about it right now, but, I wouldn't
// be surprised if changing to multi-threaded updating ends up being a massive endeavor. *shrugs*
//
//                  - sci4me, 6/4/20
//
//


// NOTE: The following crap is just to make sure
// that the vtables get initialized, which happens
// sometime during the ctor...
// Long-term, we should probably just remove all
// use of virtual methods.

template<typename T>
T* make_tile() {
    void *t = mlc_alloc(sizeof(T));
    return new(t) T;
}

Tile* make_tile(Tile_Type type) {
    Tile *result;

    switch(type) {
        case TILE_CHEST: result = make_tile<Tile_Chest>(); break;
        case TILE_FURNACE: result = make_tile<Tile_Furnace>(); break;
        case TILE_MINING_MACHINE: result = make_tile<Tile_Mining_Machine>(); break;
        case TILE_TUBE: result = make_tile<Tile_Tube>(); break;
        default: assert(0); break;
    }
    result->type = type;

    return result;
}