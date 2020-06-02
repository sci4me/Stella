namespace assets {
    #define _TEXTURES(X) \
        X(stone) \
        X(grass) \
        X(cobblestone) \
        X(coal_ore) \
        X(iron_ore) \
        X(gold_ore) \
        X(chest) \
        X(furnace) \
        X(mining_machine) \
        X(iron_plate) \
        X(gold_plate) \
        X(iron_gear)

    namespace textures {
        #define _X(name) Texture name;
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
        #define _X(name) texs[i++] = name;
        _TEXTURES(_X)
        #undef _X
    }

    // TODO(URGENT): REMOVE THIS! OH MY GOD! GLOBALS! Sure is time to pay the piper smh...
    void restore_textures(u8 *buf) {
        using namespace textures;

        Texture *texs = (Texture*) buf;

        u32 i = 0;
        #define _X(name) name = texs[i++];
        _TEXTURES(_X)
        #undef _X
    }


    void init() {
        using namespace textures;

        constexpr bool MIPS = true;

        stone                 = load_texture_from_file("assets/textures/tile/stone.png", MIPS);
        grass                 = load_texture_from_file("assets/textures/tile/grass.png", MIPS);
        cobblestone           = load_texture_from_file("assets/textures/tile/cobblestone.png", MIPS);
        coal_ore              = load_texture_from_file("assets/textures/tile/coal_ore.png", MIPS);
        iron_ore              = load_texture_from_file("assets/textures/tile/iron_ore.png", MIPS);
        gold_ore              = load_texture_from_file("assets/textures/tile/gold_ore.png", MIPS);
        chest                 = load_texture_from_file("assets/textures/tile/chest.png", MIPS);
        furnace               = load_texture_from_file("assets/textures/tile/furnace.png", MIPS);
        mining_machine        = load_texture_from_file("assets/textures/tile/mining_machine.png", MIPS);

        // NOTE: Do we actually want MIPs for items? i.e. for item entities?
        iron_plate            = load_texture_from_file("assets/textures/item/iron_plate.png", false);
        gold_plate            = load_texture_from_file("assets/textures/item/gold_plate.png", false);
        iron_gear             = load_texture_from_file("assets/textures/item/iron_gear.png", false);
    }

    void deinit() {
        using namespace textures;

        stone.deinit();
        grass.deinit();
        cobblestone.deinit();
        coal_ore.deinit();
        iron_ore.deinit();
        gold_ore.deinit();
        chest.deinit();
        furnace.deinit();
        mining_machine.deinit();

        iron_plate.deinit();
        gold_plate.deinit();
        iron_gear.deinit();
    }
}