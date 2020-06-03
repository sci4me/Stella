// TODO: Clean this up!
// We probably want to just have a macro
// for items in item.hpp and a macro for
// tiles in tile.hpp which has the id, name,
// type, etc.

#define _TEXTURES(X) \
    X(TILE_STONE,               stone,                true ,          tile,     tile  ) \
    X(TILE_GRASS,               grass,                true ,          tile,     tile  ) \
    X(TILE_COBBLESTONE,         cobblestone,          true ,          tile,     tile  ) \
    X(TILE_COAL_ORE,            coal_ore,             true ,          tile,     tile  ) \
    X(TILE_IRON_ORE,            iron_ore,             true ,          tile,     tile  ) \
    X(TILE_GOLD_ORE,            gold_ore,             true ,          tile,     tile  ) \
    X(TILE_CHEST,               chest,                true ,          tile,     tile  ) \
    X(TILE_FURNACE,             furnace,              true ,          tile,     tile  ) \
    X(TILE_MINING_MACHINE,      mining_machine,       true ,          tile,     tile  ) \
    \
    X(ITEM_COBBLESTONE,         cobblestone,          false,          item,     tile  ) \
    X(ITEM_COAL_ORE,            coal_ore,             false,          item,     tile  ) \
    X(ITEM_IRON_ORE,            iron_ore,             false,          item,     tile  ) \
    X(ITEM_GOLD_ORE,            gold_ore,             false,          item,     tile  ) \
    X(ITEM_IRON_PLATE,          iron_plate,           false,          item,     item  ) \
    X(ITEM_GOLD_PLATE,          gold_plate,           false,          item,     item  ) \
    X(ITEM_IRON_GEAR,           iron_gear,            false,          item,     item  ) \
    X(ITEM_CHEST,               chest,                false,          item,     tile  ) \
    X(ITEM_FURNACE,             furnace,              false,          item,     tile  ) \
    X(ITEM_MINING_MACHINE,      mining_machine,       false,          item,     tile  )


struct Assets {
    Texture tile_textures[N_TILE_TYPES];
    Texture item_textures[N_ITEM_TYPES];

    void init() {
        #define _X(id, name, mips, type, pfx) type##_textures[id] = load_texture_from_file(#pfx, #name, mips);
        _TEXTURES(_X)
        #undef _X
    }

    void deinit() {
        #define _X(id, name, mips, type, pfx) type##_textures[id].deinit();
        _TEXTURES(_X)
        #undef _X
    }
};