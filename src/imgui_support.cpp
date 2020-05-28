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
        io.BackendFlags = ImGuiBackendFlags_RendererHasVtxOffset;
        // io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendPlatformName = "Stella_imsupoort";
        io.BackendRendererName = "Stella_imsupoort";

        io.KeyMap[ImGuiKey_Tab] = VK_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
        io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = VK_PAGE_UP;
        io.KeyMap[ImGuiKey_PageDown] = VK_PAGE_DOWN;
        io.KeyMap[ImGuiKey_Home] = VK_HOME;
        io.KeyMap[ImGuiKey_End] = VK_END;
        io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
        io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = VK_BACKSPACE;
        io.KeyMap[ImGuiKey_Space] = VK_SPACE;
        io.KeyMap[ImGuiKey_Enter] = VK_ENTER;
        io.KeyMap[ImGuiKey_Escape] = VK_ESC;
        io.KeyMap[ImGuiKey_KeyPadEnter] = VK_KP_ENTER;
        io.KeyMap[ImGuiKey_A] = VK_A;
        io.KeyMap[ImGuiKey_C] = VK_C;
        io.KeyMap[ImGuiKey_V] = VK_V;
        io.KeyMap[ImGuiKey_X] = VK_X;
        io.KeyMap[ImGuiKey_Y] = VK_Y;
        io.KeyMap[ImGuiKey_Z] = VK_Z;

        /*
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

        // TODO: textual input?! So far we don't need it (iirc) but we probably will!!

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

    void begin_frame(PlatformIO *pio) {
        ImGuiIO& io = ImGui::GetIO();
        IM_ASSERT(io.Fonts->IsBuilt());

        io.DisplaySize = vec2(pio->window_width, pio->window_height);

        io.MousePos = vec2(pio->mouse_x, pio->mouse_y);
        io.MouseDown[ImGuiMouseButton_Left] = pio->is_button_down(VMB_LEFT);
        io.MouseDown[ImGuiMouseButton_Middle] = pio->is_button_down(VMB_MIDDLE);
        io.MouseDown[ImGuiMouseButton_Right] = pio->is_button_down(VMB_RIGHT);
        io.MouseWheelH += pio->mouse_wheel_x;
        io.MouseWheel += pio->mouse_wheel_y;

        for(u32 i = 0; i < ImGuiKey_COUNT; i++) {
            s32 j = io.KeyMap[i];
            io.KeysDown[j] = pio->is_button_down(j);
        }

        io.KeyCtrl = pio->is_button_down(VK_CTRL_LEFT) || pio->is_button_down(VK_CTRL_RIGHT);
        io.KeyShift = pio->is_button_down(VK_SHIFT_LEFT) || pio->is_button_down(VK_SHIFT_RIGHT);
        io.KeyAlt = pio->is_button_down(VK_ALT_LEFT) || pio->is_button_down(VK_ALT_RIGHT);

        #ifdef _WIN32
            io.KeySuper = false;
        #else
            io.KeySuper = pio->is_button_down(VK_SUPER_LEFT) || pio->is_button_down(VK_SUPER_RIGHT);
        #endif

        // TODO: DisplayFramebufferScale (if we ever care...)

        // TODO: DeltaTime (we DEFINITELY care about this one lol)

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
            GLenum last_clip_origin = 0; glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)&last_clip_origin);
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