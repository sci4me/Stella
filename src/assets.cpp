namespace assets {
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

    namespace textures {
        #define _X(name, mips, type) Texture name;
        _TEXTURES(_X)
        #undef _X
    }

    // TODO(URGENT): REMOVE THIS! OH MY GOD! GLOBALS! Sure is time to pay the piper smh...
    void save_textures(u8 *buf) {
        using namespace textures;
        
        // buffer overflow? more like muffler overflow.
        // ... er... uh.. shit.
        Texture *texs = (Texture*) buf;

        u32 i = 0;
        #define _X(name, mips, type) texs[i++] = name;
        _TEXTURES(_X)
        #undef _X
    }

    // TODO(URGENT): REMOVE THIS! OH MY GOD! GLOBALS! Sure is time to pay the piper smh...
    void restore_textures(u8 *buf) {
        using namespace textures;

        Texture *texs = (Texture*) buf;

        u32 i = 0;
        #define _X(name, mips, type) name = texs[i++];
        _TEXTURES(_X)
        #undef _X
    }


    void init() {
        using namespace textures;

        #define _X(name, mips, type) name = load_texture_from_file(#type, #name, mips);
        _TEXTURES(_X)
        #undef _X
    }

    void deinit() {
        using namespace textures;

        #define _X(name, mips, type) name.deinit();
        _TEXTURES(_X)
        #undef _X
    }
}