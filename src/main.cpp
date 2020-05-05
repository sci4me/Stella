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

f32 randf32() {
    return (f32)rand() / (f32)RAND_MAX;
}

GLuint load_texture(const char *path) {
    s32 w, h, n;
    u8 *image = stbi_load(path, &w, &h, &n, 0);    

    GLenum internal_format;
    GLenum data_format;
    switch(n) {
        case 3:
            internal_format = GL_RGB8;
            data_format = GL_RGB;
            break;
        case 4:
            internal_format = GL_RGBA8;
            data_format = GL_RGBA;
            break;
        default:
            assert(0);
    }

    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);

    glTextureStorage2D(texture, 1, internal_format, w, h);
    glTextureSubImage2D(texture, 0, 0, 0, w, h, data_format, GL_UNSIGNED_BYTE, image);

    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return texture;
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

    auto quad_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    GLuint t_test = load_texture("res/textures/test.png");
    GLuint t_stone = load_texture("res/textures/stone.png");
    GLuint t_grass = load_texture("res/textures/grass.png");

    struct Q {
        glm::vec2 pos;
        glm::vec2 size;
        f32 speed;
        GLuint texture;
        bool has_texture;
    };

    constexpr u32 N_QS = 10;
    Q qs[N_QS];

    #define SPAWN_Q(i) {\
            Q *q = &qs[i];\
            q->pos = glm::vec2(randf32() * 1280, -200);\
            q->size = glm::vec2(10 + randf32() * 100, 10 + randf32() * 100);\
            q->speed = (0.2f + (randf32() * 0.5f)) * 5.0f;\
            q->texture = randf32() < 0.5f ? t_stone : t_grass;\
            q->has_texture = randf32() < 0.6f;\
        }

    for(u32 i = 0; i < N_QS; i++) SPAWN_Q(i);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);

        batch_renderer_begin_frame(r);

        for(u32 i = 0; i < N_QS; i++) {
            Q *q = &qs[i];

            if(q->pos.y > 720) {
                SPAWN_Q(i);
            } else {
                q->pos.y += q->speed;
            }

            if(q->has_texture) {
                batch_renderer_push_textured_quad(r, q->pos.x, q->pos.y, q->size.x, q->size.y, q->texture);
            } else {
                batch_renderer_push_solid_quad(r, q->pos.x, q->pos.y, q->size.x, q->size.y, quad_color);                
            }
        }

        batch_renderer_end_frame(r);

        // imgui rendering pass
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
                ImGui::SetNextWindowSize(ImVec2(120.0f, 60.0f));
                ImGui::Begin("Shape Control", 0, ImGuiWindowFlags_NoResize);
                ImGui::ColorEdit4("Color", glm::value_ptr(quad_color), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_PickerHueWheel);
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