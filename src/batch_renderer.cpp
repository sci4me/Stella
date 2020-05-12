struct Batch_Renderer {
    #pragma pack(push, 1)
    struct Vertex {
        glm::vec2 pos;
        glm::vec4 color;
        glm::vec2 uv;
        s32 tex;
    };
    #pragma pack(pop)

    struct Per_Frame_Stats {
        u32 quads;
        u32 vertices;
        u32 indices;
        u32 draw_calls;
    };

    static const u32 MAX_VERTICES = 1024 * 128;
    static const u32 MAX_INDICES = MAX_VERTICES * 3;
    static const u32 MAX_TEXTURE_SLOTS = 16; // TODO

private:
    GLuint shader;
    GLint u_textures;
    GLint u_proj;
    GLint u_view;

    GLuint textures[MAX_TEXTURE_SLOTS];
    GLuint white_texture;
    u32 texture_count;

    GLuint vao;
    GLuint vbo;
    GLuint ibo;

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
        glTextureParameteri(white_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(white_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(white_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(white_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ibo);

        glBindVertexArray(vao);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex), 0, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, pos));
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, color));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, uv));
        glVertexAttribIPointer(3, 1, GL_INT, sizeof(Vertex), (void*) offsetof(Vertex, tex));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(u32), 0, GL_DYNAMIC_DRAW);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void free() {
        glDeleteProgram(shader);
        glDeleteTextures(1, &white_texture);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ibo);
    }

    void set_projection(glm::mat4 proj) {
        glProgramUniformMatrix4fv(shader, u_proj, 1, GL_FALSE, glm::value_ptr(proj));
    }

    void flush() {
        if(vertices.count == 0) return;

        assert(indices.count % 3 == 0 && indices.count > 0);

        per_frame_stats.vertices += vertices.count;
        per_frame_stats.indices += indices.count;
        per_frame_stats.draw_calls++;

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.count * sizeof(Vertex), &vertices.data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.count * sizeof(u32), &indices.data);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glUseProgram(shader);
        glBindVertexArray(vao);

        for(u32 i = 0; i < texture_count; i++)
            glBindTextureUnit(i, textures[i]);

        glDrawElements(GL_TRIANGLES, indices.count, GL_UNSIGNED_INT, 0);

        for(u32 i = 0; i < texture_count; i++)
            glBindTextureUnit(i, 0);

        glBindVertexArray(0);
        glUseProgram(0);

        begin();
    }

    void ensure_available(u32 v, u32 i) {
        assert(v < MAX_VERTICES);
        assert(i < MAX_INDICES);
        if(vertices.count + v > MAX_VERTICES || indices.count + i > MAX_INDICES) flush();
    }

    void begin(glm::mat4 view_matrix) {
        glProgramUniformMatrix4fv(shader, u_view, 1, GL_FALSE, glm::value_ptr(view_matrix));
        begin();
    }

    void begin() {
        vertices.clear();
        indices.clear();
        texture_count = 0;
    }

    void end() {
        if(vertices.count > 0) flush();
    }

    Per_Frame_Stats end_frame() {
        end();
        auto stats = per_frame_stats;
        memset(&per_frame_stats, 0, sizeof(Per_Frame_Stats));
        return stats;
    }

    void push_quad(f32 x, f32 y, f32 w, f32 h, glm::vec4 color, glm::vec2 uvs[4], GLuint texture) {
        ensure_available(4, 6);

        s32 tex_index = 0;
        if(texture) {
            for(u32 i = 1; i < texture_count; i++) {
                if(textures[i] == texture) {
                    tex_index = i;
                    break;
                }
            }
            if(!tex_index) {
                if(texture_count < MAX_TEXTURE_SLOTS) {
                    textures[texture_count] = texture;
                    tex_index = texture_count;
                    texture_count++;
                } else {
                    assert(0); // TODO
                }
            }
        }

        u32 tl = vertices.push({ {x,     y    }, color, uvs[0], tex_index });
        u32 tr = vertices.push({ {x + w, y    }, color, uvs[1], tex_index });
        u32 br = vertices.push({ {x + w, y + h}, color, uvs[2], tex_index });
        u32 bl = vertices.push({ {x,     y + h}, color, uvs[3], tex_index });

        indices.push(tl);
        indices.push(tr);
        indices.push(br);
        indices.push(br);
        indices.push(bl);
        indices.push(tl);

        per_frame_stats.quads++;
    }

    void push_quad(f32 x, f32 y, f32 w, f32 h, glm::vec4 color, GLuint texture) {
        glm::vec2 uvs[] = {
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f)
        };
        push_quad(x, y, w, h, color, uvs, texture);
    }

    void push_solid_quad(f32 x, f32 y, f32 w, f32 h, glm::vec4 color) {
        push_quad(x, y, w, h, color, white_texture);
    }

    void push_textured_quad(f32 x, f32 y, f32 w, f32 h, GLuint texture) {
        push_quad(x, y, w, h, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), texture);
    }

    void push_textured_quad(f32 x, f32 y, f32 w, f32 h, Texture *texture) {
        push_quad(x, y, w, h, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), texture->id);
    }

    void push_textured_quad(f32 x, f32 y, f32 w, f32 h, Texture_Atlas *atlas, u32 id) {
        auto entry = atlas->entries[id];
        push_quad(x, y, w, h, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), entry.uvs, atlas->id);
    }
};