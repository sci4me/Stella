struct Batch_Renderer {
    #pragma pack(push, 1)
    struct Vertex {
        vec2 pos;
        vec4 color;
        vec2 uv;
        s32 tex;
    };
    #pragma pack(pop)

    struct Per_Frame_Stats {
        u32 quads;
        u32 vertices;
        u32 indices;
        u32 draw_calls;
    };

    static constexpr u32 MAX_VERTICES = 1024 * 128;
    static constexpr u32 MAX_INDICES = MAX_VERTICES * 3;
    static constexpr u32 MAX_TEXTURE_SLOTS = 16; // TODO

private:
    GLuint shader;
    GLint u_textures;
    GLint u_proj;
    GLint u_view;

    GLuint white_texture;
    Slot_Allocator<GLuint, MAX_TEXTURE_SLOTS - 1> textures;

    Vertex_Array vao;
    Vertex_Buffer vbo;
    Index_Buffer ibo;

    Static_Array<Vertex, MAX_VERTICES> vertices;
    Static_Array<u32, MAX_INDICES> indices;

    Per_Frame_Stats per_frame_stats;
public:

    void init() {
        shader = load_shader_program("batch", VERTEX_SHADER | FRAGMENT_SHADER);
        u_textures = glGetUniformLocation(shader, "u_textures");
        u_proj = glGetUniformLocation(shader, "u_proj");
        u_view = glGetUniformLocation(shader, "u_view");


        s32 samplers[MAX_TEXTURE_SLOTS];
        for(s32 i = 0; i < MAX_TEXTURE_SLOTS; i++) 
            samplers[i] = i;
        glProgramUniform1iv(shader, u_textures, MAX_TEXTURE_SLOTS, samplers);


        glCreateTextures(GL_TEXTURE_2D, 1, &white_texture);
        glTextureStorage2D(white_texture, 1, GL_RGBA8, 1, 1);
        u32 white = 0xFFFFFFFF;
        glTextureSubImage2D(white_texture, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &white);
        glTextureParameteri(white_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(white_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(white_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(white_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);


        vbo.init(sizeof(vertices.data), GL_DYNAMIC_DRAW);
        ibo.init(sizeof(indices.data), GL_DYNAMIC_DRAW);
        
        vao.init();
        vao.add_vertex_buffer(
            vbo,
            Vertex_Element(GL_FLOAT, 2),
            Vertex_Element(GL_FLOAT, 4),
            Vertex_Element(GL_FLOAT, 2),
            Vertex_Element(GL_INT, 1)
        );
        vao.set_index_buffer(ibo);
    }

    void deinit() {
        glDeleteProgram(shader);
        glDeleteTextures(1, &white_texture);
        vao.deinit();
        vbo.deinit();
        ibo.deinit();
    }

    void set_projection(mat4 proj) {
        glProgramUniformMatrix4fv(shader, u_proj, 1, GL_FALSE, proj.value_ptr());
    }

    void flush() {
        TIMED_FUNCTION();

        if(vertices.count == 0) return;

        assert(indices.count % 3 == 0 && indices.count > 0);

        per_frame_stats.vertices += vertices.count;
        per_frame_stats.indices += indices.count;
        per_frame_stats.draw_calls++;

        vbo.set_data(&vertices.data, vertices.count * sizeof(Vertex));
        ibo.set_data(&indices.data, indices.count * sizeof(u32));

        glUseProgram(shader);
        vao.bind();

        for(u32 i = 0; i < textures.count; i++)
           glBindTextureUnit(i, textures.slots[i]);

        glDrawElements(GL_TRIANGLES, indices.count, GL_UNSIGNED_INT, 0);

        for(u32 i = 0; i < textures.count; i++)
            glBindTextureUnit(i, 0);

        vao.unbind();
        glUseProgram(0);

        begin();
    }

    void ensure_available(u32 v, u32 i) {
        assert(v < MAX_VERTICES);
        assert(i < MAX_INDICES);
        if(vertices.count + v > MAX_VERTICES || indices.count + i > MAX_INDICES) flush();
    }

    void begin(mat4 view_matrix) {
        glProgramUniformMatrix4fv(shader, u_view, 1, GL_FALSE, view_matrix.value_ptr());
        begin();
    }

    void begin() {
        vertices.clear();
        indices.clear();
        textures.clear();
    }

    void end() {
        if(vertices.count > 0) flush();
    }

    Per_Frame_Stats end_frame() {
        end();
        auto stats = per_frame_stats;
        mlc_memset(&per_frame_stats, 0, sizeof(Per_Frame_Stats));
        return stats;
    }

    void push_quad(f32 x, f32 y, f32 w, f32 h, vec4 color, vec2 uvs[4], GLuint texture) {
        TIMED_FUNCTION();
        
        ensure_available(4, 6);

        s32 tex_index;
        if(glIsTexture(texture)) tex_index = textures.alloc(texture);
        else                     tex_index = textures.alloc(white_texture);
        assert(tex_index != -1); // TODO

        auto apm_color = alpha_premultiply(color);
        u32 tl = vertices.push({ {x,     y    }, apm_color, uvs[0], tex_index });
        u32 tr = vertices.push({ {x + w, y    }, apm_color, uvs[1], tex_index });
        u32 br = vertices.push({ {x + w, y + h}, apm_color, uvs[2], tex_index });
        u32 bl = vertices.push({ {x,     y + h}, apm_color, uvs[3], tex_index });

        indices.push(tl);
        indices.push(tr);
        indices.push(br);
        indices.push(br);
        indices.push(bl);
        indices.push(tl);

        per_frame_stats.quads++;
    }

    void push_quad(f32 x, f32 y, f32 w, f32 h, vec4 color, GLuint texture) {
        vec2 uvs[] = {
            vec2(0.0f, 0.0f),
            vec2(1.0f, 0.0f),
            vec2(1.0f, 1.0f),
            vec2(0.0f, 1.0f)
        };
        push_quad(x, y, w, h, color, uvs, texture);
    }

    void push_solid_quad(f32 x, f32 y, f32 w, f32 h, vec4 color) {
        push_quad(x, y, w, h, color, white_texture);
    }

    void push_textured_quad(f32 x, f32 y, f32 w, f32 h, GLuint texture) {
        push_quad(x, y, w, h, vec4(1.0f, 1.0f, 1.0f, 1.0f), texture);
    }

    void push_textured_quad(f32 x, f32 y, f32 w, f32 h, Texture *texture) {
        push_quad(x, y, w, h, vec4(1.0f, 1.0f, 1.0f, 1.0f), texture->id);
    }
};