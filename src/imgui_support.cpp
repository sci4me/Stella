namespace imsupport {
    GLuint shader;
    GLuint u_proj;
    GLuint u_texture;

    Texture font_texture;

    Vertex_Array vao;
    Vertex_Buffer vbo;
    Index_Buffer ibo;

    void create_fonts_texture() {
        ImGuiIO& io = ImGui::GetIO();

        u8 *pixels;
        s32 width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        font_texture.init(width, height);
        font_texture.set_data(pixels);

        io.Fonts->TexID = (ImTextureID)(intptr_t)font_texture.id;
    }

    void init() {
        IMGUI_CHECKVERSION();

        // NOTE TODO: We may need this for hot code reloading;
        // currently we have a hacked up version of imgui, and,
        // we actually tell it what to use for malloc/free. So.
        // ImGui::SetAllocatorFunctions(alloc_fn, free_fn, user_data);

        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        // io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
        io.BackendPlatformName = "Stella_imsupoort";

        /*
        io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
        io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
        io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
        io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
        io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
        io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
        io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
        io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
        io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
        io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
        io.KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
        io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
        io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
        io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
        io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
        io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
        io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;
        
        io.SetClipboardTextFn = ImGui_ImplGlfw_SetClipboardText;
        io.GetClipboardTextFn = ImGui_ImplGlfw_GetClipboardText;
        io.ClipboardUserData = g_Window;
    #if defined(_WIN32)
        io.ImeWindowHandle = (void*)glfwGetWin32Window(g_Window);
    #endif

        g_MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        g_MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
        g_MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
        g_MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        g_MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    #if GLFW_HAS_NEW_CURSORS
        g_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
        g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
        g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
        g_MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
    #else
        g_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        g_MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    #endif
        */

        ImGui::StyleColorsDark();

        shader = load_shader_program("imgui", VERTEX_SHADER | FRAGMENT_SHADER);
        u_proj = glGetUniformLocation(shader, "u_proj");
        u_texture = glGetUniformLocation(shader, "u_texture");

        create_fonts_texture();

        vao.init();
        vbo.init();
        ibo.init();

        vao.add_vertex_buffer(
            vbo,
            Vertex_Element(GL_FLOAT, 2),
            Vertex_Element(GL_FLOAT, 2),
            Vertex_Element(GL_UNSIGNED_BYTE, 4, true)
        );
        vao.set_index_buffer(ibo);
    }

    void deinit() {
        ImGui::DestroyContext();

        glDeleteProgram(shader);

        font_texture.deinit();

        vao.deinit();
        vbo.deinit();
        ibo.deinit();
    }

    void mouse_position_callback(s32 x, s32 y, bool valid) {
        ImGuiIO& io = ImGui::GetIO();
        if(valid) io.MousePos = vec2(x, y);
        else      io.MousePos = vec2(-FLT_MAX, -FLT_MAX);
    }

    void mouse_button_callback(s32 button, bool is_press) {
        static constexpr s32 mouse_map[5] = { 4, 0, 2, 1, 5 };

        ImGuiIO& io = ImGui::GetIO();
        if(button >= 0 && button < array_length(mouse_map)) {
            io.MouseDown[mouse_map[button]] = is_press;
        }
    }

    void scroll_callback(f64 deltaX, f64 deltaY) {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseWheelH += (f32)deltaX;
        io.MouseWheel += (f32)deltaY;
    }

    void key_callback(s32 keycode, bool is_press) {
        ImGuiIO& io = ImGui::GetIO();
        io.KeysDown[keycode] = is_press;
        // TODO: ctrl, shift, alt, super
    }

    void begin_frame(PlatformIO *pio) {
        ImGuiIO& io = ImGui::GetIO();
        IM_ASSERT(io.Fonts->IsBuilt());

        io.DisplaySize = vec2(pio->window_width, pio->window_height);

        ImGui::NewFrame();
    }

    void end_frame() {
        ImGui::Render();
        auto dd = ImGui::GetDrawData();

        f32 l = dd->DisplayPos.x;
        f32 r = dd->DisplayPos.x + dd->DisplaySize.x;
        f32 t = dd->DisplayPos.y;
        f32 b = dd->DisplayPos.y + dd->DisplaySize.y;
        mat4 proj = mat4::ortho(l, r, b, t, -1.0f, 1.0f);
        glProgramUniformMatrix4fv(shader, u_proj, 1, GL_FALSE, proj.value_ptr());
        glProgramUniform1i(shader, u_texture, 0);

        bool clip_origin_lower_left = true;
    #if defined(GL_CLIP_ORIGIN) && !defined(__APPLE__)
        GLenum last_clip_origin = 0; glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)&last_clip_origin); // Support for GL 4.5's glClipControl(GL_UPPER_LEFT)
        if (last_clip_origin == GL_UPPER_LEFT)
            clip_origin_lower_left = false;
    #endif

        s32 fb_width = (s32)(dd->DisplaySize.x * dd->FramebufferScale.x);
        s32 fb_height = (s32)(dd->DisplaySize.y * dd->FramebufferScale.y);
        if(fb_width <= 0 || fb_height <= 0) return;

        vec2 clip_off = dd->DisplayPos;
        vec2 clip_scale = dd->FramebufferScale;

        GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
        GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
        GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
        GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
        GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
        GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_SCISSOR_TEST);
        glUseProgram(shader);

        vao.bind();

        for(s32 i = 0; i < dd->CmdListsCount; i++) {
            ImDrawList const* cmd_list = dd->CmdLists[i];

            vbo.set_data(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), GL_STREAM_DRAW);
            ibo.set_data(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), GL_STREAM_DRAW);
        
            for(s32 j = 0; j < cmd_list->CmdBuffer.Size; j++) {
                ImDrawCmd const* cmd = &cmd_list->CmdBuffer[j];

                if(cmd->UserCallback) {
                    // TODO
                    assert(0);
                } else {
                    f32 cx = (cmd->ClipRect.x - clip_off.x) * clip_scale.x;
                    f32 cy = (cmd->ClipRect.y - clip_off.y) * clip_scale.y;
                    f32 cw = (cmd->ClipRect.z - clip_off.x) * clip_scale.x;
                    f32 ch = (cmd->ClipRect.w - clip_off.y) * clip_scale.y;

                    if(cx < fb_width && cy < fb_height && cw >= 0 && ch >= 0) {
                        if(clip_origin_lower_left)  glScissor(cx, fb_height - ch, cw - cx, ch - cy);
                        else                        glScissor(cx, cy, cw, ch);

                        glBindTextureUnit(0, (GLuint)(intptr_t) cmd->TextureId);

                        glDrawElementsBaseVertex(
                            GL_TRIANGLES, 
                            cmd->ElemCount,
                            sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, 
                            (void*)(intptr_t)(cmd->IdxOffset * sizeof(ImDrawIdx)), 
                            cmd->VtxOffset
                        );
                    }
                }
            }    
        }

        vao.unbind();

        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
        glDisable(GL_SCISSOR_TEST);

        glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
        glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
    }
}