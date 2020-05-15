struct Player {
    static constexpr f32 SPEED = 10.0f;
    static constexpr f32 SIZE = 10.0f;

    GLFWwindow *window;

    World *world;
    glm::vec2 pos;

    bool tile_hovered = false;
    s32 hovered_tile_x;
    s32 hovered_tile_y;
    bool placement_valid = false;

    bool is_mining = false;
    f32 mining_progress;

    Item_Container inventory;
    crafting::Queue crafting_queue;

    Tile *active_ui_tile = nullptr;

    bool show_inventory = false;

    // TODO: maybe change this to be an overlay that we always
    // display whenever arrlen(queue) > 0?
    bool show_crafting_queue = false;

    void init(GLFWwindow *window, World *world) {
        this->window = window;
        this->world = world;

        inventory.init(16);
        crafting_queue.init(&inventory);
    }

    void free() {
        inventory.free();
        crafting_queue.deinit();
    }


    // TODO: Extract this collision code... once we have it all working.

    struct Hit {
        bool hit;
        f32 h;
        glm::vec2 n;
    };

    Hit sweep(AABB const& a, AABB const& b, glm::vec2 const& vel) {
        f32 inv_x_entry;
        f32 inv_y_entry;
        f32 inv_x_exit;
        f32 inv_y_exit;

        if(vel.x > 0.0f) {
            inv_x_entry = b.min.x - a.max.x;
            inv_x_exit = b.max.x - a.min.x;
        } else {
            inv_x_entry = b.max.x - a.min.x;
            inv_x_exit = b.min.x - a.max.x;
        }

        if(vel.y > 0.0f) {
            inv_y_entry = b.min.y - a.max.y;
            inv_y_exit = b.max.y - a.min.y;
        } else {
            inv_y_entry = b.max.y - a.min.y;
            inv_y_exit = b.min.y - a.max.y;
        }

        f32 x_entry;
        f32 y_entry;
        f32 x_exit;
        f32 y_exit;

        if(vel.x == 0.0f) {
            x_entry = -FLT_MAX;
            x_exit = FLT_MAX;
        } else {
            x_entry = inv_x_entry / vel.x;
            x_exit = inv_x_exit / vel.x;
        }

        if(vel.y == 0.0f) {
            y_entry = -FLT_MAX;
            y_exit = FLT_MAX;
        } else {
            y_entry = inv_y_entry / vel.y;
            y_exit = inv_y_exit / vel.y;
        }

        f32 entry = max(x_entry, y_entry);
        f32 exit = min(x_exit, y_exit);

        if((entry > exit) || ((x_entry < 0.0f) && (y_entry < 0.0f)) || (x_entry > 1.0f) || (y_entry > 1.0f)) {
            return { false, 1.0f };
        }

        glm::vec2 normal = {
            x_entry > y_entry ? -sign(vel.x) : 0,
            x_entry > y_entry ? 0 : -sign(vel.y)
        };
        return { true, entry, normal };
    }

    void update() {
        crafting_queue.update();

        // NOTE: We set these, if necessary, every update.
        tile_hovered = false;
        placement_valid = false;
        is_mining = false;

        ImGuiIO& io = ImGui::GetIO(); 


        if(!io.WantCaptureKeyboard) {
            glm::vec2 dir = {0, 0};
            if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) dir.y = -1.0f;
            if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) dir.y = 1.0f;
            if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) dir.x = -1.0f;
            if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) dir.x = 1.0f;
            dir = glm::normalize(dir);
            
            if(glm::length(dir) > 0) {
                auto vel = dir * SPEED;
                if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) vel = dir * 0.5f;
                
                f32 half_size = 0.5f * SIZE;
                glm::vec2 v_half_size = { half_size, half_size };
                AABB player_bb = AABB::from_center(pos, v_half_size);

                Hit best = { false, 1.0f };

                // NOTE TODO: We don't really have to check EVERY tile
                // for collisions! Just check the ones close to us.
                auto chunk = get_current_chunk();
                for(u32 i = 0; i < hmlen(chunk->layer2); i++) {
                    auto tile = chunk->layer2[i].value;
                    if(tile->flags & TILE_FLAG_IS_COLLIDER == 0) continue;

                    auto& tile_bb = tile->collision_aabb;
                    auto broad = player_bb.add(AABB::from_center(pos + vel * best.h, v_half_size));
                    if(tile_bb.intersects(broad)) {
                        static u32 blah = 0;
                        auto hit = sweep(player_bb, tile_bb, vel);
                        if(hit.hit && hit.h < best.h) {
                            best = hit;
                        }    
                    }
                }

                pos += vel * best.h;
                
                if(best.hit) {
                    f32 r = 1.0f - best.h;
                    f32 d = (vel.x * best.n.y + vel.y * best.n.x) * r;
                    pos += glm::vec2(best.n.y, best.n.x) * d;
                }
            }
    

            if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) open_inventory();


            static bool last_c_key = false;
            if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
                if(!last_c_key) {
                    last_c_key = true;
                    show_crafting_queue = !show_crafting_queue;                    
                }
            } else {
                last_c_key = false;
            }

            // TODO escape should un-hold a held item?
        }


        if(!io.WantCaptureMouse) {
            f64 mx, my;
            glfwGetCursorPos(window, &mx, &my);

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
                    
                    if(ui::held_item_container) {
                        auto held_stack = &ui::held_item_container->slots[ui::held_item_index];

                        auto wtf = hmgeti(chunk->layer2, key);
                        if(wtf != -1) {
                            placement_valid = false;
                        } else {
                            placement_valid = true;

                            // TODO: Instead of polling the mouse, we want to do this stuff
                            // in a mouse event handler!
                            if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                                Tile *tile;

                                if(held_stack->type == ITEM_CHEST) {
                                    auto tile_mem = malloc(sizeof(Tile_Chest));
                                    Tile_Chest *chest = new(tile_mem) Tile_Chest;
                                    chest->type = TILE_CHEST;
                                    tile = chest;
                                } else if(held_stack->type == ITEM_FURNACE) {
                                    auto tile_mem = malloc(sizeof(Tile_Furnace));
                                    Tile_Furnace *furnace = new(tile_mem) Tile_Furnace;
                                    furnace->type = TILE_FURNACE;
                                    tile = furnace;
                                } else {
                                    assert(0);
                                }

                                tile->x = hovered_tile_x;
                                tile->y = hovered_tile_y;
                                tile->init();
                                hmput(chunk->layer2, key, tile);

                                
                                Item_Stack stack = *held_stack;
                                assert(ui::held_item_container->remove({ stack.type, 1 }));
                                if(stack.count == 1) {
                                    // NOTE: We removed the last one in this slot/stack.

                                    auto inv = ui::held_item_container;
                                    ui::held_item_container = nullptr;
                                    for(u32 i = 0; i < inv->size; i++) {
                                        if(inv->slots[i].type == stack.type) {
                                            ui::held_item_container = inv;
                                            ui::held_item_index = i;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        auto l1i = hmgeti(chunk->layer1, key);
                        auto l2i = hmgeti(chunk->layer2, key);

                        if(l1i != -1 || l2i != -1) {
                            tile_hovered = true;

                            // TODO: Instead of polling the mouse, we want to do this stuff
                            // in a mouse event handler!
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
        if(placement_valid) {
            r->push_solid_quad(
                hovered_tile_x * TILE_SIZE, 
                hovered_tile_y * TILE_SIZE, 
                TILE_SIZE, 
                TILE_SIZE, 
                glm::vec4(1.0f, 1.0f, 1.0f, 0.2f)
            ); 
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
        ui::player_inventory(&crafting_queue, &show_inventory);

        if(show_crafting_queue) crafting_queue.draw();

        f32 half_size = SIZE / 2.0f;
        r->push_solid_quad(pos.x - half_size, pos.y - half_size, SIZE, SIZE, { 1.0f, 0.0f, 0.0f, 1.0f });
    }

private:
    Chunk* get_current_chunk() {
        return world->get_chunk_containing(floor(pos.x / TILE_SIZE), floor(pos.y / TILE_SIZE));
    }

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
            case TILE_COBBLESTONE:
            case TILE_COAL_ORE:
            case TILE_IRON_ORE:
            case TILE_GOLD_ORE: {
                Tile_Ore *ore = (Tile_Ore*) tile;

                Item_Type ore_item;
                switch(tile->type) {
                    case TILE_COBBLESTONE:  ore_item = ITEM_COBBLESTONE; break;
                    case TILE_COAL_ORE:     ore_item = ITEM_COAL_ORE; break;
                    case TILE_IRON_ORE:     ore_item = ITEM_IRON_ORE; break;
                    case TILE_GOLD_ORE:     ore_item = ITEM_GOLD_ORE; break;
                    default: assert(0);
                }

                // NOTE TODO: If the result is >0 we need to
                // either spawn the item in the world, or,
                // probably just don't actually perform 
                // the mining operation.
                //              - sci4me, 5/13/20
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