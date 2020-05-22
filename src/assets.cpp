namespace assets {
    namespace textures {
        Texture stone;
        Texture grass;
        Texture cobblestone;
        Texture coal_ore;
        Texture iron_ore;
        Texture gold_ore;
        Texture chest;
        Texture furnace;
        Texture mining_machine;

        // NOTE TODO: Do we want to separate textures for tiles, items, etc.
        // into their own namespaces? Dunno. Maybe.
        Texture iron_plate;
        Texture gold_plate;
        Texture iron_gear;
    }

    void init() {
        constexpr bool MIPS = true;

        textures::stone                 = load_texture_from_file("res/textures/tile/stone.png", MIPS);
        textures::grass                 = load_texture_from_file("res/textures/tile/grass.png", MIPS);
        textures::cobblestone           = load_texture_from_file("res/textures/tile/cobblestone.png", MIPS);
        textures::coal_ore              = load_texture_from_file("res/textures/tile/coal_ore.png", MIPS);
        textures::iron_ore              = load_texture_from_file("res/textures/tile/iron_ore.png", MIPS);
        textures::gold_ore              = load_texture_from_file("res/textures/tile/gold_ore.png", MIPS);
        textures::chest                 = load_texture_from_file("res/textures/tile/chest.png", MIPS);
        textures::furnace               = load_texture_from_file("res/textures/tile/furnace.png", MIPS);
        textures::mining_machine        = load_texture_from_file("res/textures/tile/mining_machine.png", MIPS);

        // TODO: Do we actually want MIPs for items? i.e. for item entities?
        textures::iron_plate            = load_texture_from_file("res/textures/item/iron_plate.png", false);
        textures::gold_plate            = load_texture_from_file("res/textures/item/gold_plate.png", false);
        textures::iron_gear             = load_texture_from_file("res/textures/item/iron_gear.png", false);
    }

    void deinit() {
        // TODO
    }
}