#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define SCI_H_IMPL
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

void scroll_callback(GLFWwindow *window, f64 x, f64 y) {
    scale = clampf(scale + y * 0.05f, 0.5f, 5.0f);
}

s32 main(s32 argc, char **argv) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialized GLFW3!\n");
        return 1;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO
    glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
    glfwWindowHint(GLFW_SAMPLES, 4);
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

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW!\n");
        glfwTerminate();
        return 1;
    }

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
    batch_renderer_init(r);
    batch_renderer_set_projection(r, proj);

    glClearColor(0.2, 0.1, 0.5, 1);

    GLuint t_test = load_texture("res/textures/test.png");
    GLuint t_stone = load_texture("res/textures/stone.png");
    GLuint t_grass = load_texture("res/textures/grass.png");

    f32 x = 0;
    f32 y = 0;

    World world;
    world_init(&world);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);

        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) y -= 10.0f;
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) y += 10.0f;
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) x -= 10.0f;
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) x += 10.0f;

        batch_renderer_set_scale(r, scale);

        batch_renderer_begin_frame(r);

        s32 vp_min_x = (s32) floor((x - 640.0f / scale) / TILE_SIZE);
        s32 vp_min_y = (s32) floor((y - 360.0f / scale) / TILE_SIZE);
        s32 vp_max_x = (s32) ceil((x + 640.0f / scale) / TILE_SIZE);
        s32 vp_max_y = (s32) ceil((y + 360.0f / scale) / TILE_SIZE);

        s32 vp_min_cx = (s32) floor((f32)vp_min_x / (f32)CHUNK_SIZE);
        s32 vp_min_cy = (s32) floor((f32)vp_min_y / (f32)CHUNK_SIZE);
        s32 vp_max_cx = (s32) ceil((f32)vp_max_x / (f32)CHUNK_SIZE);
        s32 vp_max_cy = (s32) ceil((f32)vp_max_y / (f32)CHUNK_SIZE);

        for(s32 i = vp_min_cx; i < vp_max_cx; i++) {
            for(s32 j = vp_min_cy; j < vp_max_cy; j++) {
                Chunk *c = world_get_chunk(&world, i, j);

                for(s32 k = 0; k < CHUNK_SIZE; k++) {
                    for(s32 l = 0; l < CHUNK_SIZE; l++) {
                        f32 tx = ((i * CHUNK_SIZE) + k) * (f32)TILE_SIZE - x;
                        f32 ty = ((j * CHUNK_SIZE) + l) * (f32)TILE_SIZE - y;

                        if(tx + TILE_SIZE < (-640.0f / scale) || 
                            ty + TILE_SIZE < (-360.0f / scale) || 
                            tx - TILE_SIZE > (640.0f / scale) || 
                            ty - TILE_SIZE > (360.0f / scale)) 
                            continue;

                        GLuint tex;
                        switch(c->tiles[k][l]) {
                            case TILE_STONE: tex = t_stone; break;
                            case TILE_GRASS: tex = t_grass; break;
                            default: assert(0); break;
                        }
                        batch_renderer_push_textured_quad(r, tx, ty, TILE_SIZE, TILE_SIZE, tex);
                    }
                }
            }
        }

        batch_renderer_push_solid_quad(r, -5, -5, 10, 10, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

        batch_renderer_end_frame(r);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            {
                ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always, ImVec2(0.0f, 0.0f));
                ImGui::SetNextWindowBgAlpha(0.35f);
                ImGui::Begin("Metrics", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
                {
                    ImGui::Text("Metrics");
                    
                    ImGui::Separator();
                    
                    ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);
                    ImGui::Text("FPS: %.1f", io.Framerate);
                    
                    ImGui::Separator();

                    auto stats = r->per_frame_stats;
                    ImGui::Text("Quads: %u", stats.quads);
                    ImGui::Text("Vertices: %u", stats.vertices);
                    ImGui::Text("Indices: %u", stats.indices);
                    ImGui::Text("Textures: %u", stats.textures);
                    ImGui::Text("Draw Calls: %u", stats.draw_calls);
                }
                ImGui::End();
            }

            {
                ImGui::SetNextWindowPos(ImVec2(10.0f, 180.0f), ImGuiCond_Always, ImVec2(0.0f, 0.0f));
                ImGui::SetNextWindowBgAlpha(0.35f);
                ImGui::Begin("Data", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
                {
                    ImGui::Text("Scale: %0.3f\n", scale);
                }
                ImGui::End();
            }
        }
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
        glfwSwapBuffers(window);
    }

    world_free(&world);

    batch_renderer_free(r);
    free(r);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

    return 0;
}