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
    Item_Container backpack; // TODO REMOVEME TESTING

    bool placing_chest = false; // TODO REMOVEME TESTING
    bool placement_valid;

    void init(GLFWwindow *window, World *world) {
        this->window = window;
        this->world = world;

        inventory.init(16);
        backpack.init(3); // TODO REMOVEME TESTING
    }

    void free() {
        inventory.free();
        backpack.free(); // TODO REMOVEME TESTING
    }

    void update() {
        glm::vec2 dir = {0, 0};
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) dir.y = -1.0f;
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) dir.y = 1.0f;
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) dir.x = -1.0f;
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) dir.x = 1.0f;
        if(glm::length(dir) > 0) pos += glm::normalize(dir) * 10.0f;


        placing_chest = glfwGetKey(window, GLFW_KEY_C); // TODO REMOVEME TESTING


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
                
                if(placing_chest) {
                    auto wtf = hmgeti(chunk->layer2, key);
                    if(wtf != -1) {
                        placement_valid = false;
                    } else {
                        placement_valid = true;

                        if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                            auto tile_mem = malloc(sizeof(Tile_Chest));
                            auto tile = new(tile_mem) Tile_Chest;
                            tile->type = TILE_CHEST;
                            tile->x = hovered_tile_x;
                            tile->y = hovered_tile_y;
                            hmput(chunk->layer2, key, tile);
                        }
                    }
                } else {
                    auto l1i = hmgeti(chunk->layer1, key);
                    auto l2i = hmgeti(chunk->layer2, key);

                    if(l1i != -1 || l2i != -1) {
                        tile_hovered = true;

                        is_mining = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
                        if(is_mining) handle_mining();
                    }
                }
            }
        }
        if(!is_mining) mining_progress = 0.0f;
    }

    void draw(Batch_Renderer *r) {
        if(placing_chest) {
            r->push_textured_quad(
                hovered_tile_x * TILE_SIZE, 
                hovered_tile_y * TILE_SIZE, 
                TILE_SIZE,
                TILE_SIZE,
                tile_textures[TILE_CHEST].id
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

        r->push_solid_quad(pos.x - 5, pos.y - 5, 10, 10, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    }

    void show_inventory() {
        ui::inventory("Inventory", &inventory, 4, 4);
        ui::inventory("Backpack", &backpack, 3, 1); // TODO REMOVEME TESTING
    }

private:
    void handle_mining() {
        // TODO: Base this on mining speed and
        // make sure to handle time correctly.
        mining_progress += fast_mining ? 0.1f : 0.015f;
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
            case TILE_COAL_ORE: {
                Tile_Ore *ore = (Tile_Ore*) tile;
                
                // TODO: we mined the ore, we ought to get that ore
                // in our inventory. Or drop it into the world as
                // an entity if our inventory is full.

                // TODO REMOVEME TESTING
                inventory.insert({ ITEM_COAL_ORE, 1 }); // NOTE: shouldn't ignore return value!

                if(ore->count == 1) {
                    hmdel(layer, key);
                } else {
                    ore->count--;
                }
                break;
            }
            default: {
                assert(0);
                break;
            }
        }
    }
};