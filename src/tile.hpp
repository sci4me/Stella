#ifndef TILE_H
#define TILE_H

constexpr f32 TILE_SIZE = 32.0f;


// NOTE TODO: Mining time!
#define _TILE_DEFS(X, _X) \
    X(_X, TILE_STONE,               stone               ) \
    X(_X, TILE_GRASS,               grass               ) \
    X(_X, TILE_COBBLESTONE,         cobblestone         ) \
    X(_X, TILE_COAL_ORE,            coal_ore            ) \
    X(_X, TILE_IRON_ORE,            iron_ore            ) \
    X(_X, TILE_GOLD_ORE,            gold_ore            ) \
    X(_X, TILE_CHEST,               chest               ) \
    X(_X, TILE_FURNACE,             furnace             ) \
    X(_X, TILE_MINING_MACHINE,      mining_machine      ) 
#define TILE_DEFS(X) _TILE_DEFS(_IGNORE__X, X)


typedef u8 Tile_Type;
enum Tile_Type_ : Tile_Type {
    #define _X(id, texname) id,
    TILE_DEFS(_X)
    #undef _X

    N_TILE_TYPES
};


typedef u8 Tile_Flags;
enum Tile_Flags_ : Tile_Flags {
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

    virtual void init();
    virtual void deinit();
    virtual void draw(Batch_Renderer *r);
    virtual void update();
};

#endif