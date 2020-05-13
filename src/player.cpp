struct Player {
    GLFWwindow *window;

    World *world;
    glm::vec2 pos;

    bool tile_hovered = false;
    s32 hovered_tile_x;
    s32 hovered_tile_y;

    bool is_mining = false;
    f32 mining_progress;

    Item_Container<4, 4> inventory;

    void init(GLFWwindow *window, World *world) {
        this->window = window;
        this->world = world;

        inventory.init();
    }

    void update() {
        glm::vec2 dir = {0, 0};
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) dir.y = -1.0f;
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) dir.y = 1.0f;
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) dir.x = -1.0f;
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) dir.x = 1.0f;
        if(glm::length(dir) > 0) pos += glm::normalize(dir) * 10.0f;


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
                
                auto l1i = hmgeti(chunk->layer1, key);
                auto l2i = hmgeti(chunk->layer2, key);

                if(l1i != -1 || l2i != -1) {
                    tile_hovered = true;

                    is_mining = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
                    if(is_mining) handle_mining();
                }
            }
        }
        if(!is_mining) mining_progress = 0.0f;
    }

    void draw(Batch_Renderer *r) {
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
        ui::inventory("Inventory", &inventory);
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