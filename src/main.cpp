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

#define GL_DEBUG

constexpr f64 PI = 3.14159265358979323846;

char* read_entire_file(char *name) {
    FILE *fp = fopen(name, "rb");
    if (!fp) {
        return 0;
    }

    fseek(fp, 0L, SEEK_END);
    u32 size = ftell(fp);
    rewind(fp);

    char *code = (char*) malloc(size + 1);

    // TODO: use fread
    for (u32 i = 0; i < size; i++) {
        code[i] = fgetc(fp);
    }
    code[size] = 0;

    return code;
}

void program_attach_shader(GLuint program, char *name, GLenum type) {
    const char *source = read_entire_file(name);

    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &source, 0);
    glCompileShader(id);

    GLint success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint info_log_length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_log_length);

        GLchar *info_log = (GLchar*) alloca(sizeof(GLchar) * (info_log_length + 1));
        glGetShaderInfoLog(id, info_log_length, 0, info_log);

        fprintf(stderr, "Error compiling shader `%s`:\n%s\n", name, info_log);
        exit(1);
    }

    glAttachShader(program, id);
}

enum {
    VERTEX_SHADER = 1,
    GEOMETRY_SHADER = 2,
    FRAGMENT_SHADER = 4
};

GLuint load_program(const char *name, GLenum flags) {
    GLuint p = glCreateProgram();

    char buffer[1024];

    #define SHADER_TYPE(t, n, s)\
        if(flags & t) {\
            sprintf(buffer, "res/shaders/%s/%s.glsl", name, n);\
            program_attach_shader(p, buffer, s);\
        }

    SHADER_TYPE(VERTEX_SHADER, "vert", GL_VERTEX_SHADER)
    SHADER_TYPE(GEOMETRY_SHADER, "geom", GL_GEOMETRY_SHADER)
    SHADER_TYPE(FRAGMENT_SHADER, "frag", GL_FRAGMENT_SHADER)

    #undef SHADER_TYPE

    GLint success;
    glLinkProgram(p);
    glGetProgramiv(p, GL_LINK_STATUS, &success);
    if (!success) {
        GLint info_log_length;
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &info_log_length);

        GLchar *info_log = (GLchar*)alloca(sizeof(GLchar) * (info_log_length + 1));
        glGetProgramInfoLog(p, info_log_length, 0, info_log);

        fprintf(stderr, "%s\n", info_log);
        exit(1);
    }

    return p;
}

template<u32 segments = 32>
void generate_disc_vertices(f32 *a, f32 cx, f32 cy, f32 r) {
    static_assert(segments >= 3);

    f32 half_r = r / 2.0f;
    f32 theta = 2.0f * (f32)PI / ((f32)segments);
    f32 c = cosf(theta);
    f32 s = sinf(theta);

    f32 x = r;
    f32 y = 0;

    f32 *ptr = a;
    for (u32 i = 0; i <= segments; i++) {
        *(ptr++) = cx + x;
        *(ptr++) = cy + y;

        f32 t = x;
        x = c * x - s * y;
        y = s * t + c * y;
    }
}

#ifdef GL_DEBUG
void gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION || severity == GL_DEBUG_SEVERITY_LOW) {
        return;
    }

    printf("%s\n", message);
    fflush(stdout);
}
#endif

s32 main(s32 argc, char **argv) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialized GLFW3!\n");
        return 1;
    }

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
    glm::mat4 proj = glm::ortho(0.0f, 1920.0f, 1080.0f, 0.0f, 0.0f, 10000.0f); // TODO: we need to update this for the shader any time a resize occurs

    // load and enable our shader program
    GLuint program = load_program("basic", VERTEX_SHADER | FRAGMENT_SHADER);

    glUseProgram(program);
    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(proj));
    glUseProgram(0);

    // generate the vertices for our disc
    constexpr u32 N = 64;
    float vertices[(N + 1) * 2];
    generate_disc_vertices<N>(vertices, 500.0f, 500.0f, 100.0f);

    // create and set up VBO and VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    
    glBindVertexArray(vao);
    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glClearColor(0.2, 0.1, 0.5, 1);

    f32 my_color[4] = { 1, 0, 0, 1 };

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);

        // render our mesh
        glUseProgram(program);
        {
            glUniform4fv(1, 1, my_color);

            glBindVertexArray(vao);

            glDrawArrays(GL_TRIANGLE_FAN, 0, (N + 1) * 2);

            glBindVertexArray(0);
        }
        glUseProgram(0);

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
                }
                ImGui::End();
            }

            {
                ImGui::SetNextWindowSize(ImVec2(120.0f, 60.0f));
                ImGui::Begin("Shape Control", 0, ImGuiWindowFlags_NoResize);
                ImGui::ColorEdit4("Color", my_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_PickerHueWheel);
                ImGui::End();
            }
        }
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

    return 0;
}