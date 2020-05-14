namespace assets {
    namespace textures {
        Texture stone;
        Texture grass;
        Texture coal_ore[7]; // TODO: Don't hardcode this 7
        Texture iron_ore[7]; // TODO: Don't hardcode this 7
        Texture chest;
        Texture furnace;
    }

    void load() {
        constexpr bool MIPS = true;

        textures::stone           = load_texture_from_file("res/textures/tile/stone.png", MIPS);
        textures::grass           = load_texture_from_file("res/textures/tile/grass.png", MIPS);
        textures::coal_ore[0]     = load_texture_from_file("res/textures/tile/coal_ore_0.png", MIPS); // TODO: use a for loop for these, lol
        textures::coal_ore[1]     = load_texture_from_file("res/textures/tile/coal_ore_1.png", MIPS);
        textures::coal_ore[2]     = load_texture_from_file("res/textures/tile/coal_ore_2.png", MIPS);
        textures::coal_ore[3]     = load_texture_from_file("res/textures/tile/coal_ore_3.png", MIPS);
        textures::coal_ore[4]     = load_texture_from_file("res/textures/tile/coal_ore_4.png", MIPS);
        textures::coal_ore[5]     = load_texture_from_file("res/textures/tile/coal_ore_5.png", MIPS);
        textures::coal_ore[6]     = load_texture_from_file("res/textures/tile/coal_ore_6.png", MIPS);
        textures::iron_ore[0]     = load_texture_from_file("res/textures/tile/iron_ore_0.png", MIPS);
        textures::iron_ore[1]     = load_texture_from_file("res/textures/tile/iron_ore_1.png", MIPS);
        textures::iron_ore[2]     = load_texture_from_file("res/textures/tile/iron_ore_2.png", MIPS);
        textures::iron_ore[3]     = load_texture_from_file("res/textures/tile/iron_ore_3.png", MIPS);
        textures::iron_ore[4]     = load_texture_from_file("res/textures/tile/iron_ore_4.png", MIPS);
        textures::iron_ore[5]     = load_texture_from_file("res/textures/tile/iron_ore_5.png", MIPS);
        textures::iron_ore[6]     = load_texture_from_file("res/textures/tile/iron_ore_6.png", MIPS);
        textures::chest           = load_texture_from_file("res/textures/tile/chest.png", MIPS);
        textures::furnace         = load_texture_from_file("res/textures/tile/furnace.png", MIPS);
    }
}