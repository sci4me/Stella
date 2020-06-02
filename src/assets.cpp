#define _TEXTURES(X) \
    X(stone,                true ,          tile  ) \
    X(grass,                true ,          tile  ) \
    X(cobblestone,          true ,          tile  ) \
    X(coal_ore,             true ,          tile  ) \
    X(iron_ore,             true ,          tile  ) \
    X(gold_ore,             true ,          tile  ) \
    X(chest,                true ,          tile  ) \
    X(furnace,              true ,          tile  ) \
    X(mining_machine,       true ,          tile  ) \
    X(iron_plate,           false,          item  ) \
    X(gold_plate,           false,          item  ) \
    X(iron_gear,            false,          item  )


struct Assets {
    #define _X(name, mips, type) Texture name;
    _TEXTURES(_X)
    #undef _X

    void init() {
        #define _X(name, mips, type) name = load_texture_from_file(#type, #name, mips);
        _TEXTURES(_X)
        #undef _X
    }

    void deinit() {
        #define _X(name, mips, type) name.deinit();
        _TEXTURES(_X)
        #undef _X
    }
};