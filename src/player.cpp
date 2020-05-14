struct Player {
    GLFWwindow *window;

    World *world;
    glm::vec2 pos;

    bool tile_hovered = false;
    s32 hovered_tile_x;
    s32 hovered_tile_y;

    bool is_mining = false;
    f32 mining_progress;

    Item_Container inventory;

    Tile *active_ui_tile = nullptr;

    bool show_inventory = false;

    Tile_Type placing_tile = TILE_NONE; // TODO
    bool placement_valid;

    void init(GLFWwindow *window, World *world) {
        this->window = window;
        this->world = world;

        inventory.init(16);
    }

    void free() {
        inventory.free();
    }

    void update() {
        ImGuiIO& io = ImGui::GetIO();


        if(!io.WantCaptureKeyboard) {
            glm::vec2 dir = {0, 0};
            if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) dir.y = -1.0f;
            if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) dir.y = 1.0f;
            if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) dir.x = -1.0f;
            if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) dir.x = 1.0f;
            if(glm::length(dir) > 0) pos += glm::normalize(dir) * 10.0f;
    
            if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) open_inventory();

            if(glfwGetKey(window, GLFW_KEY_C)) placing_tile = TILE_CHEST;
            else if(glfwGetKey(window, GLFW_KEY_F)) placing_tile = TILE_FURNACE;
            else placing_tile = TILE_NONE;
        }


        if(!io.WantCaptureMouse) {
            f64 mx, my;
            glfwGetCursorPos(window, &mx, &my);

            tile_hovered = false;
            is_mining = false;
            if(mx >= 0 && my >= 0 && mx < window_width && my < window_height) {
                glm::vec2 mouse_world_pos = {
                    pos.x + ((mx - (window_width / 2)) / scale),
                    pos.y + ((my - (window_height / 2)) / scale)
                };

                // NOTE: `10 * TILE_SIZE` is the max distance the player can "reach".
                if(glm::distance(mouse_world_pos, pos) < 10 * TILE_SIZE) {
                    hovered_tile_x = floor(mouse_world_pos.x / TILE_SIZE);
                    hovered_tile_y = floor(mouse_world_pos.y / TILE_SIZE);
                    
                    Chunk *chunk = world->get_chunk_containing(hovered_tile_x, hovered_tile_y);

                    glm::ivec2 key = {
                        hovered_tile_x & (Chunk::SIZE - 1),
                        hovered_tile_y & (Chunk::SIZE - 1)
                    };
                    
                    // NOTE TODO: Rip most of this out and such.
                    if(placing_tile != TILE_NONE) {
                        auto wtf = hmgeti(chunk->layer2, key);
                        if(wtf != -1) {
                            placement_valid = false;
                        } else {
                            placement_valid = true;

                            if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                                if(placing_tile == TILE_CHEST) {
                                    auto tile_mem = malloc(sizeof(Tile_Chest));
                                    auto tile = new(tile_mem) Tile_Chest;
                                    tile->init();
                                    tile->type = TILE_CHEST;
                                    tile->x = hovered_tile_x;
                                    tile->y = hovered_tile_y;
                                    hmput(chunk->layer2, key, tile);
                                } else if(placing_tile == TILE_FURNACE) {
                                    auto tile_mem = malloc(sizeof(Tile_Furnace));
                                    auto tile = new(tile_mem) Tile_Furnace;
                                    tile->init();
                                    tile->type = TILE_FURNACE;
                                    tile->x = hovered_tile_x;
                                    tile->y = hovered_tile_y;
                                    hmput(chunk->layer2, key, tile);
                                } else {
                                    assert(0);
                                }
                            }
                        }
                    } else {
                        auto l1i = hmgeti(chunk->layer1, key);
                        auto l2i = hmgeti(chunk->layer2, key);

                        if(l1i != -1 || l2i != -1) {
                            tile_hovered = true;

                            if(l2i != -1 && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                                // NOTE TODO: This shit is fkin hardcoded asf yo. But I don't want to go through
                                // the pain of making all these fkin hpp files and .. goddamn.. uh.. forward declaring
                                // my ass. So fuck it, for now it's this way, future me will suffer through fixing it.
                                // #PrankYourFutureSelf2k20 #YEET
                                //              - sci4me, 5/13/20
                                auto tile = chunk->layer2[l2i].value;
                                switch(tile->type) {
                                    case TILE_CHEST:
                                    case TILE_FURNACE:
                                        open_tile_ui(tile);
                                        break;
                                    default:
                                        assert(0);
                                        break;
                                }
                            } else {
                                is_mining = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
                                if(is_mining) handle_mining();    
                            }
                        }
                    }
                }
            }
        }

        if(!is_mining) mining_progress = 0.0f;
    }

    void draw(Batch_Renderer *r) {
        if(placing_tile != TILE_NONE) {
            r->push_textured_quad(
                hovered_tile_x * TILE_SIZE, 
                hovered_tile_y * TILE_SIZE, 
                TILE_SIZE,
                TILE_SIZE,
                tile_textures[placing_tile].id
            );

            if(!placement_valid) {
                r->push_solid_quad(
                    hovered_tile_x * TILE_SIZE, 
                    hovered_tile_y * TILE_SIZE, 
                    TILE_SIZE,
                    TILE_SIZE,
                    glm::vec4(1.0f, 0.0f, 0.0f, 0.65f)
                );
            }
        }

        if(tile_hovered) {
            if(is_mining) {
                r->push_solid_quad(
                    hovered_tile_x * TILE_SIZE, 
                    hovered_tile_y * TILE_SIZE, 
                    clampf(mining_progress, 0.0f, 1.0f) * TILE_SIZE,
                    TILE_SIZE,
                    glm::vec4(1.0f, 0.0f, 0.0f, 0.5f)
                );
            } else {
                r->push_solid_quad(
                    hovered_tile_x * TILE_SIZE, 
                    hovered_tile_y * TILE_SIZE, 
                    TILE_SIZE, 
                    TILE_SIZE, 
                    glm::vec4(1.0f, 1.0f, 1.0f, 0.2f)
                ); 
            }
        }

        ui::tile_ui(&inventory, &active_ui_tile);
        ui::player_inventory(&inventory, &show_inventory);

        r->push_solid_quad(pos.x - 5, pos.y - 5, 10, 10, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    }

private:
    void open_inventory() {
        active_ui_tile = nullptr;
        show_inventory = true;
    }

    void open_tile_ui(Tile *t) {
        active_ui_tile = t;
        show_inventory = false;
    }

    void handle_mining() {
        // TODO: Base this on mining speed and
        // make sure to handle time correctly.
        mining_progress += fast_mining ? 0.5f : 0.015f;
        if(mining_progress >= 1.0f) {
            mining_progress = 0.0f;
        } else {
            return;
        }

        Chunk *chunk = world->get_chunk_containing(hovered_tile_x, hovered_tile_y);

        glm::ivec2 key = {
            hovered_tile_x & (Chunk::SIZE - 1),
            hovered_tile_y & (Chunk::SIZE - 1)
        };

        auto layer = chunk->layer2;
        auto index = hmgeti(layer, key);
        if(index == -1) {
            layer = chunk->layer1;
            index = hmgeti(layer, key);
        }
        assert(index != -1);
        
        Tile *tile = layer[index].value;

        switch(tile->type) {
            case TILE_COAL_ORE: 
            case TILE_IRON_ORE: {
                Tile_Ore *ore = (Tile_Ore*) tile;
                
                // NOTE TODO: If the result is >0 we need to
                // either spawn the item in the world, or,
                // probably just don't actually perform 
                // the mining operation.
                //              - sci4me, 5/13/20

                Item_Type ore_item;
                switch(tile->type) {
                    case TILE_COAL_ORE: ore_item = ITEM_COAL_ORE; break;
                    case TILE_IRON_ORE: ore_item = ITEM_IRON_ORE; break;
                    default: assert(0);
                }

                assert(!inventory.insert({ ore_item, 1 }));

                if(ore->count == 1) {
                    hmdel(layer, key);
                    ore->free();
                    ::free(ore);
                } else {
                    ore->count--;
                }
                break;
            }
            case TILE_CHEST: {
                Tile_Chest *chest = (Tile_Chest*) tile;

                // NOTE: Doing this here feels so wrong lol.
                // Almost makes me wonder if we don't want to have
                // some sort of handle system for referencing tiles
                // so that we can handle when they're destroyed
                // a little bit better... .... maybe.....
                if(active_ui_tile == chest) active_ui_tile = nullptr;

                for(u32 i = 0; i < chest->container.size; i++) {
                    if(!chest->container.slots[i].count) continue;

                    u32 rem = inventory.insert(chest->container.slots[i]);

                    // NOTE TODO: If the result is >0 we have to
                    // spawn the item in the world.
                    assert(!rem);
                }

                // TODO: remove the assert, handle the result.
                assert(!inventory.insert({ ITEM_CHEST, 1 }));

                hmdel(layer, key);
                chest->free();
                ::free(chest);
                break;
            }
            case TILE_FURNACE: {
                assert(0);
                break;
            }
            default: {
                assert(0);
                break;
            }
        }
    }
};