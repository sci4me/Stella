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

#ifdef GL_DEBUG
void gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION || severity == GL_DEBUG_SEVERITY_LOW) {
        return;
    }

    printf("%s\n", message);
    fflush(stdout);
}
#endif

bool show_metrics = true;

void key_callback(GLFWwindow *window, s32 key, s32 scancode, s32 action, s32 mods) {
    if(key == GLFW_KEY_F3 && action == GLFW_RELEASE) {
        show_metrics = !show_metrics;
    }
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

    glfwSetKeyCallback(window, key_callback);

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
    glm::mat4 proj = glm::ortho(0.0f, 1280.0f, 720.0f, 0.0f, 0.0f, 10000.0f); // TODO: we need to update this for the shader any time a resize occurs

    // set up batch renderer
    Batch_Renderer *r = (Batch_Renderer*) malloc(sizeof(Batch_Renderer));
    batch_renderer_init(r);
    batch_renderer_set_projection(r, proj);

    glClearColor(0.2, 0.1, 0.5, 1);

    GLuint t_stone = load_texture("res/textures/stone.png");
    GLuint t_grass = load_texture("res/textures/grass.png");

    siv::PerlinNoise noise;
    f32 frequency = 10.0f;
    f32 stone_threshold = 0.5f;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);

        // main rendering
        batch_renderer_begin_frame(r);

        const f32 w = 16;
        const f32 h = 16;
        for(u32 i = 0; i < 1280 / w; i++) {
            for(u32 j = 0; j < 720 / h; j++) {
                f32 k = noise.noise2D_0_1(i / frequency, j / frequency);

                GLuint tex;
                if(k < stone_threshold) tex = t_stone;
                else tex = t_grass;

                batch_renderer_push_textured_quad(r, i * w, j * h, w, h, tex);
            }
        }

        batch_renderer_end_frame(r);

        // imgui rendering pass
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            if(show_metrics) {
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
                    ImGui::Text("Vertices: %u", stats.vertices);
                    ImGui::Text("Indices: %u", stats.indices);
                    ImGui::Text("Textures: %u", stats.textures);
                    ImGui::Text("Draw Calls: %u", stats.draw_calls);
                }
                ImGui::End();
            }
            
            {
                ImGui::Begin("Controls", 0, ImGuiWindowFlags_None);
                ImGui::SliderFloat("Frequency", &frequency, 2.0f, 64.0f);
                ImGui::SliderFloat("Stone Threshold", &stone_threshold, 0.0f, 1.0f);
                ImGui::End();
            }
        }
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
        glfwSwapBuffers(window);
    }

    batch_renderer_free(r);
    free(r);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

    return 0;
}