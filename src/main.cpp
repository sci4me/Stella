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

// Set to true so that we don't have to do
// any sizing code before the main loop.
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


#define COLLISION_DEBUG
#ifdef COLLISION_DEBUG
// TODO: Combine these structs! Every time we get a
// collision using the broad AABB, we _always_ perform
// the swept collision check. So. No need for these 
// to be separate.
//              - sci4me, 5/15/20
struct Collision_Debug_Data {
    AABB broad_aabb;
    AABB collider_aabb;
    AABB player_aabb;
    glm::vec2 vel;
    AABB::Hit h;

    bool broad_aabb_selected;
    bool collider_aabb_selected;
    bool player_aabb_selected;
    bool h_selected;
};

Collision_Debug_Data *collision_debug_data_this_frame = nullptr;

void show_collision_debug_per_frame_data(glm::mat4 view) {
    auto dl = ImGui::GetForegroundDrawList();

    auto txfm = [&](glm::vec2 v) {
        auto t = view * glm::vec4(v, 0.0f, 1.0f);
        return ImVec2(t.x, t.y);
    };

    auto draw_aabb = [&](AABB const& a, glm::vec4 const& color) {
        auto min = txfm(a.min);
        auto max = txfm(a.max);
        dl->AddRect(min, max, rgba1_to_rgba255(color));
    };

    auto draw_line = [&](glm::vec2 const& start, glm::vec2 const& end, glm::vec4 const& color, f32 const thicc = 3.0f) {
        auto a = txfm(start);
        auto b = txfm(end);
        dl->AddLine(a, b, rgba1_to_rgba255(color), thicc);
    };

    for(u32 i = 0; i < arrlen(collision_debug_data_this_frame); i++) {
        auto& c = collision_debug_data_this_frame[i];

        ImGui::PushID(i);
        if(ImGui::TreeNodeEx("Collision_Debug_Data", 0, "Collision #%d", i)) {
            ImGui::PushID("player_aabb_selected");
                ImGui::Selectable("", &c.player_aabb_selected);
                ImGui::SameLine();
                ImGui::Text("Player: (%0.3f, %0.3f) -> (%0.3f, %0.3f)", c.player_aabb.min.x, c.player_aabb.min.y, c.player_aabb.max.x, c.player_aabb.max.y);
            ImGui::PopID();

            ImGui::PushID("broad_aabb_selected");
                ImGui::Selectable("", &c.broad_aabb_selected);
                ImGui::SameLine();
                ImGui::Text("Broad: (%0.3f, %0.3f) -> (%0.3f, %0.3f)", c.broad_aabb.min.x, c.broad_aabb.min.y, c.broad_aabb.max.x, c.broad_aabb.max.y);
            ImGui::PopID();

            ImGui::PushID("collider_aabb_selected");
                ImGui::Selectable("", &c.collider_aabb_selected);
                ImGui::SameLine();
                ImGui::Text("Collider: (%0.3f, %0.3f) -> (%0.3f, %0.3f)", c.collider_aabb.min.x, c.collider_aabb.min.y, c.collider_aabb.max.x, c.collider_aabb.max.y);
            ImGui::PopID();

            ImGui::PushID("h_selected");
                ImGui::Selectable("", &c.h_selected);
                ImGui::SameLine();
                ImGui::Text("Hit: %s, h: %0.9f, N: (%0.1f, %0.1f)", c.h.hit ? "true" : "false", c.h.h, c.h.n.x, c.h.n.y);
            ImGui::PopID();

            ImGui::Text("Collider ^ Player: %s", c.collider_aabb.intersects(c.player_aabb) ? "true" : "false");
            ImGui::Text("Collider ^ Broad: %s", c.collider_aabb.intersects(c.broad_aabb) ? "true" : "false");
            ImGui::Text("Velocity: (%0.3f, %0.3f)", c.vel.x, c.vel.y);

            if(c.player_aabb_selected) draw_aabb(c.player_aabb, { 0.0f, 0.0f, 1.0f, 1.0f });
            if(c.broad_aabb_selected) draw_aabb(c.broad_aabb, { 0.0f, 0.0f, 1.0f, 1.0f });
            if(c.collider_aabb_selected) draw_aabb(c.collider_aabb, { 0.0f, 0.0f, 1.0f, 1.0f });
            if(c.h_selected) assert(0); // TODO

            if(c.h.hit) {
                auto& bb = c.collider_aabb;
                auto& n = c.h.n;
                auto& start = n * bb.get_half_size() + bb.get_center();
                draw_line(start, start + n * 16.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
            }

            ImGui::TreePop();
        }
        ImGui::PopID();
    }
}
#endif


// TODO: Move this up with the rest of the includes!
// We probably want to create a struct to hold things
// like `window_width`, etc. before doing so...
//              - sci4me, 5/12/20
#include "player.cpp"


void scroll_callback(GLFWwindow *window, f64 x, f64 y) {
    ImGuiIO& io = ImGui::GetIO();
    if(!io.WantCaptureMouse) {
        scale = clampf(scale + y * 0.05f, 0.1f, 5.0f);
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


    glm::mat4 projection_matrix;


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
        player.inventory.insert({ ITEM_CHEST, MAX_ITEM_SLOT_SIZE });
        player.inventory.insert({ ITEM_FURNACE, MAX_ITEM_SLOT_SIZE });

        auto f = [&](s32 x, s32 y) {
            auto c = world.get_chunk_containing(x, y);

            auto t = new Tile_Furnace;
            t->type = TILE_FURNACE;
            t->x = x;
            t->y = y;
            t->init();

            glm::ivec2 key = {
                t->x & (Chunk::SIZE - 1),
                t->y & (Chunk::SIZE - 1)
            };
            hmput(c->layer2, key, t);
        };

        f(135, -4);
        f(135, -5);
        f(136, -5);
    }



    u32 chunk_draw_calls = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();


        if(window_resized) {
            window_resized = false;

            glViewport(0, 0, window_width, window_height);
            
            projection_matrix = glm::ortho(0.0f, (f32)window_width, (f32)window_height, 0.0f, 0.0f, 10000.0f);

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
                ImGui::Dummy(ImVec2(130, 0));

                if(ImGui::CollapsingHeader("Misc.")) {
                    ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);
                    ImGui::Text("FPS: %.1f", io.Framerate);
                }

                if(ImGui::CollapsingHeader("Player")) {
                    ImGui::Text("Position: (%0.3f, %0.3f)", player.pos.x, player.pos.y);
                    ImGui::Text("Tile Position: (%d, %d)", (s32) floor(player.pos.x / TILE_SIZE), (s32) floor(player.pos.y / TILE_SIZE));

#ifdef COLLISION_DEBUG
                    ImGui::Separator();
                    show_collision_debug_per_frame_data(view);
#endif
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