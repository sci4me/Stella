struct Batch_Vertex {
    glm::vec2 pos;
    glm::vec4 color;
    glm::vec2 uv;
    s32 tex;
};

struct Batch_Renderer_Per_Frame_Stats {
    u32 quads;
    u32 vertices;
    u32 indices;
    u32 textures;
    u32 draw_calls;
};

struct Batch_Renderer {
    static const u32 MAX_VERTICES = 1024 * 64;
    static const u32 MAX_INDICES = MAX_VERTICES * 3;
    static const u32 MAX_TEXTURE_SLOTS = 16; // TODO

    GLuint shader;
    
    GLuint textures[MAX_TEXTURE_SLOTS];
    GLuint white_texture;
    u32 texture_count;

    GLuint vao;
    GLuint vbo;
    GLuint ibo;

    Batch_Vertex vertices[MAX_VERTICES];
    u32 vertex_count;
    u32 indices[MAX_INDICES];
    u32 index_count;

    Batch_Renderer_Per_Frame_Stats per_frame_stats;
};

void batch_renderer_init(Batch_Renderer *r) {
    r->shader = load_shader_program("batch", VERTEX_SHADER | FRAGMENT_SHADER);

    s32 samplers[Batch_Renderer::MAX_TEXTURE_SLOTS];
    for(s32 i = 0; i < Batch_Renderer::MAX_TEXTURE_SLOTS; i++) 
        samplers[i] = i;
    glProgramUniform1iv(r->shader, 2, Batch_Renderer::MAX_TEXTURE_SLOTS, samplers);

    glCreateTextures(GL_TEXTURE_2D, 1, &r->white_texture);
    glTextureStorage2D(r->white_texture, 1, GL_RGBA8, 1, 1);
    u32 white = 0xFFFFFFFF;
    glTextureSubImage2D(r->white_texture, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &white);
    glTextureParameteri(r->white_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(r->white_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(r->white_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(r->white_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
    r->textures[0] = r->white_texture;
    r->texture_count = 1;

    glGenVertexArrays(1, &r->vao);
    glGenBuffers(1, &r->vbo);
    glGenBuffers(1, &r->ibo);

    glBindVertexArray(r->vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferData(GL_ARRAY_BUFFER, Batch_Renderer::MAX_VERTICES * sizeof(Batch_Vertex), 0, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Batch_Vertex), (void*) offsetof(Batch_Vertex, pos));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Batch_Vertex), (void*) offsetof(Batch_Vertex, color));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Batch_Vertex), (void*) offsetof(Batch_Vertex, uv));
    glVertexAttribIPointer(3, 1, GL_INT, sizeof(Batch_Vertex), (void*) offsetof(Batch_Vertex, tex));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Batch_Renderer::MAX_INDICES * sizeof(u32), 0, GL_DYNAMIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void batch_renderer_free(Batch_Renderer *r) {
    glDeleteProgram(r->shader);
}

void batch_renderer_set_projection(Batch_Renderer *r, glm::mat4 proj) {
    glProgramUniformMatrix4fv(r->shader, 0, 1, GL_FALSE, glm::value_ptr(proj));
}

void batch_renderer_set_scale(Batch_Renderer *r, f32 zoom) {
    glProgramUniform1f(r->shader, 1, zoom);
}

void batch_renderer_flush(Batch_Renderer *r) {
    if(r->vertex_count == 0) return;

    assert(r->index_count % 3 == 0 && r->index_count > 0);

    r->per_frame_stats.vertices += r->vertex_count;
    r->per_frame_stats.indices += r->index_count;
    r->per_frame_stats.draw_calls++;

    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, r->vertex_count * sizeof(Batch_Vertex), r->vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, r->index_count * sizeof(u32), r->indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glUseProgram(r->shader);
    glBindVertexArray(r->vao);

    for(u32 i = 0; i < r->texture_count; i++)
        glBindTextureUnit(i, r->textures[i]);

    glDrawElements(GL_TRIANGLES, r->index_count, GL_UNSIGNED_INT, 0);

    for(u32 i = 0; i < r->texture_count; i++)
        glBindTextureUnit(i, 0);

    glBindVertexArray(0);
    glUseProgram(0);

    r->vertex_count = 0;
    r->index_count = 0;
    r->texture_count = 1;
}

void batch_renderer_ensure_available(Batch_Renderer *r, u32 v, u32 i) {
    assert(v < Batch_Renderer::MAX_VERTICES);
    assert(i < Batch_Renderer::MAX_INDICES);

    if(r->vertex_count + v > Batch_Renderer::MAX_VERTICES || r->index_count + i > Batch_Renderer::MAX_INDICES) {
        batch_renderer_flush(r);
    }
}

void batch_renderer_begin(Batch_Renderer *r) {
    r->vertex_count = 0;
    r->index_count = 0;
    r->texture_count = 1;
}

void batch_renderer_end(Batch_Renderer *r) {
    if(r->vertex_count > 0) batch_renderer_flush(r);
}

Batch_Renderer_Per_Frame_Stats batch_renderer_end_frame(Batch_Renderer *r) {
    batch_renderer_end(r);
    auto stats = r->per_frame_stats;
    memset(&r->per_frame_stats, 0, sizeof(Batch_Renderer_Per_Frame_Stats));
    return stats;
}

inline u32 batch_renderer_push_vertex(Batch_Renderer *r, Batch_Vertex v) {
    r->vertices[r->vertex_count] = v;
    return r->vertex_count++;
}

inline void batch_renderer_push_index(Batch_Renderer *r, u32 i) {
    r->indices[r->index_count++] = i;
}

void batch_renderer_push_quad(Batch_Renderer *r, f32 x, f32 y, f32 w, f32 h, glm::vec4 color, glm::vec2 uvs[4], GLuint texture) {
    batch_renderer_ensure_available(r, 4, 6);

    s32 tex_index = 0;
    if(texture) {
        for(u32 i = 1; i < r->texture_count; i++) {
            if(r->textures[i] == texture) {
                tex_index = i;
                break;
            }
        }
        if(!tex_index) {
            if(r->texture_count < Batch_Renderer::MAX_TEXTURE_SLOTS) {
                r->textures[r->texture_count] = texture;
                tex_index = r->texture_count;
                r->texture_count++;
                r->per_frame_stats.textures++;
            } else {
                assert(0); // TODO
            }
        }
    }

    u32 tl = batch_renderer_push_vertex(r, {glm::vec2(x,     y    ), color, uvs[0], tex_index});
    u32 tr = batch_renderer_push_vertex(r, {glm::vec2(x + w, y    ), color, uvs[1], tex_index});
    u32 br = batch_renderer_push_vertex(r, {glm::vec2(x + w, y + h), color, uvs[2], tex_index});
    u32 bl = batch_renderer_push_vertex(r, {glm::vec2(x,     y + h), color, uvs[3], tex_index});

    batch_renderer_push_index(r, tl);
    batch_renderer_push_index(r, tr);
    batch_renderer_push_index(r, br);
    batch_renderer_push_index(r, br);
    batch_renderer_push_index(r, bl);
    batch_renderer_push_index(r, tl);

    r->per_frame_stats.quads++;
}

void batch_renderer_push_quad(Batch_Renderer *r, f32 x, f32 y, f32 w, f32 h, glm::vec4 color, GLuint texture) {
    glm::vec2 uvs[] = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f)
    };
    batch_renderer_push_quad(r, x, y, w, h, color, uvs, texture);
}

void batch_renderer_push_solid_quad(Batch_Renderer *r, f32 x, f32 y, f32 w, f32 h, glm::vec4 color) {
    batch_renderer_push_quad(r, x, y, w, h, color, 0);
}

void batch_renderer_push_textured_quad(Batch_Renderer *r, f32 x, f32 y, f32 w, f32 h, GLuint texture) {
    batch_renderer_push_quad(r, x, y, w, h, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), texture);
}

void batch_renderer_push_textured_quad(Batch_Renderer *r, f32 x, f32 y, f32 w, f32 h, Texture_Atlas *atlas, u32 id) {
    auto entry = atlas->entries[id];
    batch_renderer_push_quad(r, x, y, w, h, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), entry.uvs, atlas->id);
}