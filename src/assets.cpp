namespace assets {
    namespace textures {
        Texture stone;
        Texture grass;
        Texture cobblestone[7]; // TODO: Don't hardcode this 7
        Texture coal_ore[7]; // TODO: Don't hardcode this 7
        Texture iron_ore[7]; // TODO: Don't hardcode this 7
        Texture gold_ore[7]; // TODO: Don't hardcode this 7
        Texture chest;
        Texture furnace;

        // NOTE TODO: Do we want to separate textures for tiles, items, etc.
        // into their own namespaces? Dunno. Maybe.
        Texture iron_ingot;
        Texture gold_ingot;
    }

    void load() {
        constexpr bool MIPS = true;

        textures::stone                 = load_texture_from_file("res/textures/tile/stone.png", MIPS);
        textures::grass                 = load_texture_from_file("res/textures/tile/grass.png", MIPS);
        textures::cobblestone[0]        = load_texture_from_file("res/textures/tile/cobblestone_0.png", MIPS);
        textures::cobblestone[1]        = load_texture_from_file("res/textures/tile/cobblestone_1.png", MIPS);
        textures::cobblestone[2]        = load_texture_from_file("res/textures/tile/cobblestone_2.png", MIPS);
        textures::cobblestone[3]        = load_texture_from_file("res/textures/tile/cobblestone_3.png", MIPS);
        textures::cobblestone[4]        = load_texture_from_file("res/textures/tile/cobblestone_4.png", MIPS);
        textures::cobblestone[5]        = load_texture_from_file("res/textures/tile/cobblestone_5.png", MIPS);
        textures::cobblestone[6]        = load_texture_from_file("res/textures/tile/cobblestone_6.png", MIPS);
        textures::coal_ore[0]           = load_texture_from_file("res/textures/tile/coal_ore_0.png", MIPS); // TODO: use a for loop for these, lol
        textures::coal_ore[1]           = load_texture_from_file("res/textures/tile/coal_ore_1.png", MIPS);
        textures::coal_ore[2]           = load_texture_from_file("res/textures/tile/coal_ore_2.png", MIPS);
        textures::coal_ore[3]           = load_texture_from_file("res/textures/tile/coal_ore_3.png", MIPS);
        textures::coal_ore[4]           = load_texture_from_file("res/textures/tile/coal_ore_4.png", MIPS);
        textures::coal_ore[5]           = load_texture_from_file("res/textures/tile/coal_ore_5.png", MIPS);
        textures::coal_ore[6]           = load_texture_from_file("res/textures/tile/coal_ore_6.png", MIPS);
        textures::iron_ore[0]           = load_texture_from_file("res/textures/tile/iron_ore_0.png", MIPS);
        textures::iron_ore[1]           = load_texture_from_file("res/textures/tile/iron_ore_1.png", MIPS);
        textures::iron_ore[2]           = load_texture_from_file("res/textures/tile/iron_ore_2.png", MIPS);
        textures::iron_ore[3]           = load_texture_from_file("res/textures/tile/iron_ore_3.png", MIPS);
        textures::iron_ore[4]           = load_texture_from_file("res/textures/tile/iron_ore_4.png", MIPS);
        textures::iron_ore[5]           = load_texture_from_file("res/textures/tile/iron_ore_5.png", MIPS);
        textures::iron_ore[6]           = load_texture_from_file("res/textures/tile/iron_ore_6.png", MIPS);
        textures::gold_ore[0]           = load_texture_from_file("res/textures/tile/gold_ore_0.png", MIPS);
        textures::gold_ore[1]           = load_texture_from_file("res/textures/tile/gold_ore_1.png", MIPS);
        textures::gold_ore[2]           = load_texture_from_file("res/textures/tile/gold_ore_2.png", MIPS);
        textures::gold_ore[3]           = load_texture_from_file("res/textures/tile/gold_ore_3.png", MIPS);
        textures::gold_ore[4]           = load_texture_from_file("res/textures/tile/gold_ore_4.png", MIPS);
        textures::gold_ore[5]           = load_texture_from_file("res/textures/tile/gold_ore_5.png", MIPS);
        textures::gold_ore[6]           = load_texture_from_file("res/textures/tile/gold_ore_6.png", MIPS);
        textures::chest                 = load_texture_from_file("res/textures/tile/chest.png", MIPS);
        textures::furnace               = load_texture_from_file("res/textures/tile/furnace.png", MIPS);

        // TODO: Do we actually want MIPs for items? i.e. for item entities?
        textures::iron_ingot      = load_texture_from_file("res/textures/item/iron_ingot.png", false);
        textures::gold_ingot      = load_texture_from_file("res/textures/item/gold_ingot.png", false);
    }
}