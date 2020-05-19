// NOTE: This file depends on GL_MAJOR and GL_MINOR being defined.

void imgui_init(GLFWwindow *window) {
    IMGUI_CHECKVERSION();

    // NOTE TODO: We'll need this for hot code reloading!
    // ImGui::SetAllocatorFunctions(alloc_fn, free_fn, user_data);

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    char glsl_version_string[13];
    assert(snprintf(glsl_version_string, sizeof(glsl_version_string)/sizeof(char), "#version %d%d0", GL_MAJOR, GL_MINOR) == 12);
    ImGui_ImplOpenGL3_Init(glsl_version_string);

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
}

void imgui_shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void imgui_begin_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void imgui_end_frame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}