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


// TODO: Move this up with the rest of the includes!
// We probably want to create a struct to hold things
// like `window_width`, etc. before doing so...
//              - sci4me, 5/12/20
#include "player.cpp"


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


    load_tile_textures();


    glm::mat4 projection_matrix;


    World world;
    world.init();


    Player player;
    player.window = window;
    player.world = &world;
    player.pos = {4400, -100};


    Texture slot_texture = load_texture_from_file("res/textures/gui/slot.png");


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


        player.update();


        imgui_begin_frame();


        {
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
            r->end(); // NOTE: calling `end` instead of `end_frame`; we call `end_frame` later.
        }


        Batch_Renderer::Per_Frame_Stats per_frame_stats;
        {
            // TODO: This UI scaling scheme _mostly_ solves our problem.
            // However, there appear to be edge cases that cause undesirable
            // artifacts. If you make the window smaller, you can get to sizes
            // which cause the GUI textures to appear to be drawn incorrectly.
            // Sorry I can't think of a more specific way to explain the artifacts
            // right now but I'm not a writer :P
            // It may be something relating to the fact that we're using floating
            // point numbers to handle this scaling stuff. Or something. I don't know.
            // But eventually we'll definitely need to figure it out and fix it!!!
            //              - sci4me, 5/12/20


            // TODO: don't do this stuff per-frame
            f32 aspect = (f32)window_width / (f32)window_height;
            
            f32 v_width = 600.0f;
            f32 v_height = 600.0f; 
            if(aspect > 1) {
                v_width *= aspect;
            } else {
                v_height /= aspect;
            }

            // TODO: Using the inverse projection matrix is a bit of a hack.
            // We transform points from "GUI space" to clip space and then
            // from clip space into screen space (0-window_width, 0-window_height).
            // We then wastefully undo this last transform by going from screen
            // space back to clip space in our shader.
            //
            // Instead, we should just do the algebra and figure out what matrix
            // we need to create in order to go from "GUI space" directly to
            // screen space.
            //
            // Alternatively, we could just change the projection matrix [to the
            // "GUI view" matrix] and use an identity matrix as our view matrix.
            // Maybe this is a better way? *shrugs*
            //
            //                  - sci4me, 5/12/20
            auto gui_view = glm::ortho(0.0f, v_width, v_height, 0.0f, 0.0f, 10000.0f);
            auto view = glm::inverse(projection_matrix) * gui_view;

            r->begin(view);
            {
                s32 w2 = 18 * 2;
                s32 aw2 = w2 * 9;
                for(u32 i = 0; i < 9; i++) {
                    r->push_textured_quad(
                        (v_width / 2.0f) - (aw2 / 2.0f) + i * w2,
                        v_height - w2 - 4,
                        w2,
                        w2,
                        &slot_texture
                    );
                }


                // TODO render UI
            }
            per_frame_stats = r->end_frame();
        }


        if(show_debug_window) {
            if(ImGui::Begin("Debug Info", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav)) {
                ImGui::Dummy(ImVec2(130, 0));

                if(ImGui::CollapsingHeader("Misc.")) {
                    ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);
                    ImGui::Text("FPS: %.1f", io.Framerate);
                }

                if(ImGui::CollapsingHeader("Player")) {
                    ImGui::Text("Position: (%0.3f, %0.3f)", player.pos.x, player.pos.y);
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