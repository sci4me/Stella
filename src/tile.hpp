constexpr f32 TILE_SIZE = 32.0f;


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