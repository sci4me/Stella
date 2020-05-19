#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define SCI_H_IMPL
#define SCI_H_TEMP_STORAGE_ASSERT_NO_OVERRUN
#include "sci.h"

#define GLEW_STATIC
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui/imgui_impl_opengl3.h"

// NOTE TODO: We will need to do this for hot code reloading!
// #define STBDS_REALLOC(context,ptr,size) better_realloc
// #define STBDS_FREE(context,ptr)         better_free

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

// NOTE DODO: We will need to do this for hot code reloading!
// #define STBI_MALLOC(sz)           malloc(sz)
// #define STBI_REALLOC(p,newsz)     realloc(p,newsz)
// #define STBI_FREE(p)              free(p)

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
#include "static_bitset.cpp"
#include "slot_allocator.cpp"
#include "shader.cpp"
#include "texture.cpp"
#include "buffer_objects.cpp"
#include "texture_atlas.cpp"
#include "batch_renderer.cpp"
#include "assets.cpp"
#include "item.cpp"
#include "crafting.cpp"
#include "tile.cpp"
#include "world.cpp"
#include "ui.cpp"

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

// NOTE: Set to true so that we don't have to 
// do any sizing code before the main loop.
//          - sci4me, 5/7/20
bool window_resized = true;

f32 scale = 1.0f;

bool show_debug_window = false;
bool show_imgui_metrics_window = false;

bool fullscreen_changed = false;
bool fullscreen = false;

bool vsync = true;
bool fast_mining = false;

// TODO: Eventually we'll want to properly separate
// the "debug" / "dev" code from the production code
// using the preprocessor so we can just disable all
// of it from even being compiled. Someday TM.
//              - sci4me, 5/15/20
bool debug_pause = false;


// TODO: Move this up with the rest of the includes!
// We probably want to create a struct to hold things
// like `window_width`, etc. before doing so...
//              - sci4me, 5/12/20
#include "player.cpp"


void scroll_callback(GLFWwindow *window, f64 x, f64 y) {
    ImGuiIO& io = ImGui::GetIO();
    if(!io.WantCaptureMouse) {
        scale = clamp<f32>(scale + y * 0.05f, 0.1f, 5.0f);
    }
}

void key_callback(GLFWwindow *window, s32 key, s32 scancode, s32 action, s32 mods) {
    if(action == GLFW_RELEASE) {
        switch(key) {
            case GLFW_KEY_F3:
                show_debug_window = !show_debug_window;
                break;
            case GLFW_KEY_F11:
                fullscreen = !fullscreen;
                fullscreen_changed = true;
                break;
            case GLFW_KEY_F12:
                debug_pause = !debug_pause;
                break;
        }
    }
}

void window_size_callback(GLFWwindow* window, s32 width, s32 height) {
    window_width = width;
    window_height = height;
    window_resized = true;
}

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
        fprintf(stderr, "Failed to create GLFW window!\n");
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


    glClearColor(0, 0, 0, 0);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);


    Batch_Renderer *r = (Batch_Renderer*) malloc(sizeof(Batch_Renderer));
    r->init();


    assets::load();
    init_tiles();
    init_items();


    crafting::init();


    mat4 projection_matrix;


    World world;
    world.init();


    Player player;
    player.init(window, &world);
    player.pos = {4400, -100};



    {
        // TODO REMOVEME TESTING
        

        player.inventory.insert({ ITEM_COBBLESTONE, MAX_ITEM_SLOT_SIZE });
        player.inventory.insert({ ITEM_IRON_ORE, MAX_ITEM_SLOT_SIZE });
        player.inventory.insert({ ITEM_GOLD_ORE, MAX_ITEM_SLOT_SIZE });
        player.inventory.insert({ ITEM_COAL_ORE, MAX_ITEM_SLOT_SIZE });
        player.inventory.insert({ ITEM_IRON_PLATE, MAX_ITEM_SLOT_SIZE });
        player.inventory.insert({ ITEM_CHEST, MAX_ITEM_SLOT_SIZE });
        player.inventory.insert({ ITEM_FURNACE, MAX_ITEM_SLOT_SIZE });
        player.inventory.insert({ ITEM_MINING_MACHINE, MAX_ITEM_SLOT_SIZE });

        auto f = [&](s32 x, s32 y) {
            auto c = world.get_chunk_containing(x, y);

            auto t = new Tile_Furnace;
            t->type = TILE_FURNACE;
            t->x = x;
            t->y = y;
            t->init();

            ivec2 key = {
                t->x & (Chunk::SIZE - 1),
                t->y & (Chunk::SIZE - 1)
            };
            hmput(c->layer2, key, t);
        };

        f(135, -4);
        f(135, -5);
        f(136, -5);
    }



    f64 last_time = glfwGetTime();

    vec4 color;

    u32 chunk_draw_calls = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();


        if(window_resized) {
            window_resized = false;

            glViewport(0, 0, window_width, window_height);
            
            projection_matrix = mat4::ortho(0.0f, (f32)window_width, (f32)window_height, 0.0f, 0.0f, 10000.0f);

            r->set_projection(projection_matrix);
            world.set_projection(projection_matrix);
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


        // TODO: We don't want to just update, render, and loop;
        // we don't want to have to rely on V-SYNC to give us a fixed
        // update rate. Instead, we should keep track of time and
        // call update as needed. Or something. TM.
        //
        // Also, sooner-than-later, we're going to need to have a more
        // formal way of tracking time anyway since we need to have
        // "ticks" for our update functions for things like machines...
        // So, yeah, do this. Soon. Or something. Maybe. I guess. ???
        // (Oh yeah, uh, the thing that made me think to say this was that
        // uh, we need to be able to actually keep track of in-game time;
        // i.e. how many ticks are in a "day", what is the current tick 
        // since world creation, what day is it, is it day/night, etc.)
        //
        //              - sci4me, 5/13/20

        if(!debug_pause) {
            world.update();
            player.update();
        }


        imgui_begin_frame();


        //
        // NOTE: We only have to recompute the view matrix if:
        // 1. the scale changes
        // 2. the window size changes
        // 3. the player moves
        //
        // Since the player is probably going to be moving
        // for like 50%+ of the time, it may not be worth it
        // to cache these. *shrugs*
        // If it's an issue, SIMD-ize the matrix multiply.
        //              
        //              - sci4me, 5/18/20
        //

        auto s = mat4::scale(scale, scale, 1.0f);
        auto t = mat4::translate((window_width / 2 / scale) - player.pos.x, (window_height / 2 / scale) - player.pos.y, 0.0f);
        auto view = t * s;
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
                ImGui::ColorPicker4("Le' Color", (f32*) &color, ImGuiColorEditFlags_PickerHueWheel);

                ImGui::Dummy({ 130, 0 });

                if(ImGui::CollapsingHeader("Misc.")) {
                    ImGui::Text("FPS: %.1f", io.Framerate);
                    ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);
                }

                if(ImGui::CollapsingHeader("Player")) {
                    ImGui::Text("Position: (%0.3f, %0.3f)", player.pos.x, player.pos.y);
                    ImGui::Text("Tile Position: (%d, %d)", (s32) floor(player.pos.x / TILE_SIZE), (s32) floor(player.pos.y / TILE_SIZE));
                }

                if(ImGui::CollapsingHeader("World")) {
                    ImGui::Text("Scale: %0.3f", scale);
                    ImGui::Text("Total Chunks: %d", hmlen(world.chunks));
                    ImGui::Text("Chunk Draw Calls: %d", chunk_draw_calls);
                }

                if(ImGui::CollapsingHeader("Batch Renderer")) {
                    ImGui::Text("Quads: %u", per_frame_stats.quads);
                    ImGui::Text("Vertices: %u", per_frame_stats.vertices);
                    ImGui::Text("Indices: %u", per_frame_stats.indices);
                    ImGui::Text("Draw Calls: %u", per_frame_stats.draw_calls);
                }

                if(ImGui::CollapsingHeader("Settings")) {
                    if(ImGui::Checkbox("V-SYNC", &vsync)) {
                        if(vsync) glfwSwapInterval(1);
                        else      glfwSwapInterval(0);
                    }
                    ImGui::Checkbox("Fast Mining", &fast_mining);
                    ImGui::Checkbox("Show ImGui Metrics", &show_imgui_metrics_window);
                }
            }

            if(show_imgui_metrics_window) {
                ImGui::ShowMetricsWindow();
            }

            ImGui::End();
        }
    

        imgui_end_frame();
    
        glfwSwapBuffers(window);

        treset();
    }

    world.free();
    player.free();

    r->free();
    free(r);

    crafting::free();

    tfree();

    imgui_shutdown();

    glfwTerminate();

    return 0;
}