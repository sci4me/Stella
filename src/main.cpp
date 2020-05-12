#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define SCI_H_IMPL
#define SCI_H_TEMP_STORAGE_ASSERT_NO_OVERRUN
#include "sci.h"

#define GLEW_STATIC
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui/imgui_impl_opengl3.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "PerlinNoise.hpp"

#define RND_IMPLEMENTATION
#include "rnd.h"

#define GL_DEBUG
#define GL_MAJOR 4
#define GL_MINOR 4

#include "imgui_support.cpp"

#include "util.cpp"
#include "math.cpp"
#include "static_array.cpp"
#include "slot_allocator.cpp"
#include "shader.cpp"
#include "texture.cpp"
#include "buffer_objects.cpp"
#include "texture_atlas.cpp"
#include "batch_renderer.cpp"
#include "tile.cpp"
#include "world.cpp"

#ifdef GL_DEBUG
void gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION || severity == GL_DEBUG_SEVERITY_LOW) {
        return;
    }

    printf("%s\n", message);
    fflush(stdout);
}
#endif


s32 window_width = 1280;
s32 window_height = 720;

// Set to true so that we don't have to do
// any sizing code before the main loop.
//          - sci4me, 5/7/20
bool window_resized = true;

f32 scale = 1.0f;

bool show_debug_window = true;

bool fullscreen_changed = false;
bool fullscreen = false;

bool vsync = true;


void scroll_callback(GLFWwindow *window, f64 x, f64 y) {
    scale = clampf(scale + y * 0.05f, 0.25f, 5.0f);
}

void key_callback(GLFWwindow *window, s32 key, s32 scancode, s32 action, s32 mods) {
    if(key == GLFW_KEY_F3 && action == GLFW_RELEASE) show_debug_window = !show_debug_window;

    if(key == GLFW_KEY_F11 && action == GLFW_RELEASE) {
        fullscreen = !fullscreen;
        fullscreen_changed = true;
    }
}

void window_size_callback(GLFWwindow* window, s32 width, s32 height) {
    window_width = width;
    window_height = height;
    window_resized = true;
}


struct Player {
    GLFWwindow *window;

    World *world;
    glm::vec2 pos;

    bool tile_hovered = false;
    s32 hovered_tile_x;
    s32 hovered_tile_y;

    bool is_mining = false;
    f32 mining_progress;

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
    }

    void draw(Batch_Renderer *r) {
        if(tile_hovered) {
            // TODO: `is_mining`
            r->push_solid_quad(hovered_tile_x * TILE_SIZE, hovered_tile_y * TILE_SIZE, TILE_SIZE, TILE_SIZE, glm::vec4(1.0f, 1.0f, 1.0f, 0.2f)); 
        }

        r->push_solid_quad(pos.x - 5, pos.y - 5, 10, 10, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    }

private:
    void handle_mining() {
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

        // TODO
        switch(tile->type) {
            case TILE_COAL_ORE: {
                Tile_Ore *ore = (Tile_Ore*) tile;
                if(ore->count == 1) {
                    hmdel(layer, key);

                    // TODO: we mined the ore, we ought to get that ore
                    // in our inventory. Or drop it into the world as an entity
                    // if our inventory is full.
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


s32 main(s32 argc, char **argv) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialized GLFW3!\n");
        return 1;
    }

    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef GL_DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    #endif

    GLFWwindow *window = glfwCreateWindow(window_width, window_height, "Stella", nullptr, nullptr);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window!");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW!\n");
        glfwTerminate();
        return 1;
    }


    dump_gl_info();


    #ifdef GL_DEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback((GLDEBUGPROCARB)gl_debug_callback, 0);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
    #endif


    tinit();


    time_t t;
    srand((unsigned) time(&t));


    imgui_init(window);
    ImGuiIO& io = ImGui::GetIO();


    Batch_Renderer *r = (Batch_Renderer*) malloc(sizeof(Batch_Renderer));
    r->init();


    glClearColor(0, 0, 0, 0);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);


    load_tile_textures();


    World world;
    world.init();


    Player player;
    player.window = window;
    player.world = &world;
    player.pos = {4400, -100};


    u32 chunk_draw_calls = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();


        if(window_resized) {
            window_resized = false;

            glViewport(0, 0, window_width, window_height);
            
            auto proj = glm::ortho(0.0f, (f32)window_width, (f32)window_height, 0.0f, 0.0f, 10000.0f);

            r->set_projection(proj);
            world.set_projection(proj);
        }

        if(fullscreen_changed) {
            fullscreen_changed = false;

            static s32 wx, wy, ww, wh;
            if(fullscreen) {
                glfwGetWindowPos(window, &wx, &wy);
                glfwGetWindowSize(window, &ww, &wh);

                auto monitor = glfwGetPrimaryMonitor();
                auto mode = glfwGetVideoMode(monitor);
                glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, 0);
            } else {
                glfwSetWindowMonitor(window, nullptr, wx, wy, ww, wh, 0);
            }

            continue;
        }


        glClear(GL_COLOR_BUFFER_BIT);


        player.update();


        imgui_begin_frame();


        auto view = glm::translate(
            glm::scale(
                glm::mat4(1.0f),
                glm::vec3(scale, scale, 1.0f)
            ),
            glm::vec3((window_width / 2 / scale) - player.pos.x, (window_height / 2 / scale) - player.pos.y, 0.0f)
        );

        r->begin(view);
        {
            // NOTE: We render the world from within the Batch_Renderer frame since
            // we are currently using the Batch_Renderer for any tiles that
            // aren't on layer 0.
            //              - sci4me, 5/9/20
            chunk_draw_calls = world.draw_around(r, player.pos, scale, window_width, window_height, view);

            player.draw(r);  
        }
        auto per_frame_stats = r->end_frame();


        if(show_debug_window) {
            if(ImGui::Begin("Debug Info", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav)) {
                ImGui::Dummy(ImVec2(100, 0));

                ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);
                ImGui::Text("FPS: %.1f", io.Framerate);

                if(ImGui::CollapsingHeader("Batch Renderer")) {
                    ImGui::Text("Quads: %u", per_frame_stats.quads);
                    ImGui::Text("Vertices: %u", per_frame_stats.vertices);
                    ImGui::Text("Indices: %u", per_frame_stats.indices);
                    ImGui::Text("Draw Calls: %u", per_frame_stats.draw_calls);
                }

                if(ImGui::CollapsingHeader("World")) {
                    ImGui::Text("Position: (%0.3f, %0.3f)", player.pos.x, player.pos.y);
                    ImGui::Text("Scale: %0.3f", scale);
                    ImGui::Text("Total Chunks: %d", hmlen(world.chunks));
                    ImGui::Text("Chunk Draw Calls: %d", chunk_draw_calls);
                }

                if(ImGui::CollapsingHeader("Settings")) {
                    if(ImGui::Checkbox("V-SYNC", &vsync)) {
                        if(vsync) glfwSwapInterval(1);
                        else      glfwSwapInterval(0);
                    }
                }
            }
            ImGui::End();
        }
    

        imgui_end_frame();
    
        glfwSwapBuffers(window);

        treset();
    }

    world.free();

    r->free();
    free(r);

    tfree();

    imgui_shutdown();

    glfwTerminate();

    return 0;
}