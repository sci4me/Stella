#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define SCI_H_IMPL
#define SCI_H_TEMP_STORAGE_ASSERT_NO_OVERRUN
#include "sci.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

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

#define GL_DEBUG

#include "util.cpp"
#include "shader.cpp"
#include "texture_atlas.cpp"
#include "batch_renderer.cpp"
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

f32 scale = 1.0f;
bool show_debug_info = false;

void scroll_callback(GLFWwindow *window, f64 x, f64 y) {
    scale = clampf(scale + y * 0.05f, 0.25f, 5.0f);
}

void key_callback(GLFWwindow *window, s32 key, s32 scancode, s32 action, s32 mods) {
    if(key == GLFW_KEY_F3 && action == GLFW_RELEASE) show_debug_info = !show_debug_info;
}

s32 main(s32 argc, char **argv) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialized GLFW3!\n");
        return 1;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO
    glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef GL_DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    #endif

    GLFWwindow *window = glfwCreateWindow(1280, 720, "Stella", nullptr, nullptr);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window!");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW!\n");
        glfwTerminate();
        return 1;
    }


    tinit();


    time_t t;
    srand((unsigned) time(&t));


    #ifdef GL_DEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback((GLDEBUGPROCARB)gl_debug_callback, 0);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
    #endif


    IMGUI_CHECKVERSION(); // TODO ?
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 440"); // TODO

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark(); // default but do it anyway


    // create our projection matrix
    // glm::mat4 proj = glm::ortho(0.0f, 1280.0f, 720.0f, 0.0f, 0.0f, 10000.0f); // TODO: we need to update this for the shader any time a resize occurs
    glm::mat4 proj = glm::ortho(-640.0f, 640.0f, 360.0f, -360.0f, 0.0f, 10000.0f);

    // set up batch renderer
    Batch_Renderer *r = (Batch_Renderer*) malloc(sizeof(Batch_Renderer));
    r->init();
    r->set_projection(proj);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    glClearColor(0.2, 0.1, 0.5, 1);
    glViewport(0, 0, 1280, 720); // TODO: resize


    load_tile_textures();


    glm::vec2 pos = {0, 0};

    World world;
    world.init();

    TileType selected_tile_type = N_TILE_TYPES;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);


        glm::vec2 dir = {0, 0};
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) dir.y = -1.0f;
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) dir.y = 1.0f;
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) dir.x = -1.0f;
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) dir.x = 1.0f;
        if(glm::length(dir) > 0) pos += glm::normalize(dir) * 10.0f;

        if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) selected_tile_type = TILE_STONE;
        else if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) selected_tile_type = TILE_GRASS;


        r->set_scale(scale);
        r->begin();
        {
            world.render_around_player(r, pos, scale);

            if(selected_tile_type != N_TILE_TYPES) {
                f64 mx, my;
                glfwGetCursorPos(window, &mx, &my);

                f32 i = pos.x + (mx - 640.0f) / scale;
                f32 j = pos.y + (my - 360.0f) / scale;

                s32 k = floor(i / TILE_SIZE);
                s32 l = floor(j / TILE_SIZE);

                if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                    world.set_tile(k, l, selected_tile_type);
                }

                f32 m = k * TILE_SIZE - pos.x;
                f32 n = l * TILE_SIZE - pos.y;

                r->push_solid_quad(m, n, TILE_SIZE, TILE_SIZE, glm::vec4(1.0f, 1.0f, 0.0f, 0.5f));
                r->push_textured_quad(m + TILE_SIZE/4, n + TILE_SIZE/4, TILE_SIZE/2, TILE_SIZE/2, tile_textures[(u32)selected_tile_type]);
            }

            r->push_solid_quad(-5, -5, 10, 10, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        }
        auto per_frame_stats = r->end_frame();


        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if(show_debug_info) {
            ImGui::SetNextWindowBgAlpha(0.5f);
            if(ImGui::Begin("Debug Info", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav)) {
                ImGui::Dummy(ImVec2(100, 0));

                if(ImGui::CollapsingHeader("Metrics")) {
                    ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);
                    ImGui::Text("FPS: %.1f", io.Framerate);
                    
                    ImGui::Separator();

                    ImGui::Text("Quads: %u", per_frame_stats.quads);
                    ImGui::Text("Vertices: %u", per_frame_stats.vertices);
                    ImGui::Text("Indices: %u", per_frame_stats.indices);
                    ImGui::Text("Textures: %u", per_frame_stats.textures);
                    ImGui::Text("Draw Calls: %u", per_frame_stats.draw_calls);
                }

                if(ImGui::CollapsingHeader("World")) {
                    ImGui::Text("Position: (%0.3f, %0.3f)", pos.x, pos.y);
                    ImGui::Text("Scale: %0.3f", scale);
                    ImGui::Text("Chunks: %d", hmlen(world.chunks));
                }
            }
            ImGui::End();
        }
    
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
        glfwSwapBuffers(window);

        treset();
    }

    world.free();

    tfree();

    r->free();
    free(r);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

    return 0;
}