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
    u32 draw_calls;
};

struct Batch_Renderer {
    static const u32 MAX_QUADS = 1024 * 32;
    static const u32 MAX_VERTICES = MAX_QUADS * 4;
    static const u32 MAX_INDICES = MAX_QUADS * 6;
    static const u32 MAX_TEXTURE_SLOTS = 8; // TODO

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
    glProgramUniform1iv(r->shader, 1, Batch_Renderer::MAX_TEXTURE_SLOTS, samplers);

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(r->vertices), 0, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Batch_Vertex), (void*) offsetof(Batch_Vertex, pos));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Batch_Vertex), (void*) offsetof(Batch_Vertex, color));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Batch_Vertex), (void*) offsetof(Batch_Vertex, uv));
    glVertexAttribPointer(3, 1, GL_INT, GL_FALSE, sizeof(Batch_Vertex), (void*) offsetof(Batch_Vertex, tex));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(r->indices), 0, GL_DYNAMIC_DRAW);

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

void batch_renderer_flush(Batch_Renderer *r) {
    if(r->vertex_count == 0) return;

    r->per_frame_stats.vertices += r->vertex_count;
    r->per_frame_stats.indices += r->index_count;
    r->per_frame_stats.draw_calls++;

    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, r->vertex_count * sizeof(Batch_Vertex), r->vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, r->index_count * sizeof(Batch_Vertex), r->indices);
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
}

void batch_renderer_begin_frame(Batch_Renderer *r) {
    r->vertex_count = 0;
    r->index_count = 0;

    memset(&r->per_frame_stats, 0, sizeof(Batch_Renderer_Per_Frame_Stats));
}

void batch_renderer_end_frame(Batch_Renderer *r) {
    if(r->vertex_count > 0) batch_renderer_flush(r);
}

#define PUSH_VERTEX(p, c, u, t) r->vertices[r->vertex_count++] = {p, c, u, t};
#define PUSH_INDEX(i) r->indices[r->index_count++] = i;

void batch_renderer_push_quad(Batch_Renderer *r, f32 x, f32 y, f32 w, f32 h, glm::vec4 color) {
    // TODO: the condition of if statement is stupid
    if(r->vertex_count + 4 > Batch_Renderer::MAX_VERTICES || r->index_count + 6 > Batch_Renderer::MAX_INDICES) {
        batch_renderer_flush(r);
    }

    r->per_frame_stats.quads++;

    PUSH_VERTEX(glm::vec2(x, y + h), color, glm::vec2(), 0);
    PUSH_VERTEX(glm::vec2(x + w, y + h), color, glm::vec2(), 0);
    PUSH_VERTEX(glm::vec2(x, y), color, glm::vec2(), 0);
    PUSH_VERTEX(glm::vec2(x + w, y), color, glm::vec2(), 0);

    auto tl = r->vertex_count - 4;
    auto tr = r->vertex_count - 3;
    auto bl = r->vertex_count - 2;
    auto br = r->vertex_count - 1;

    PUSH_INDEX(tl);
    PUSH_INDEX(bl);
    PUSH_INDEX(tr);
    PUSH_INDEX(bl);
    PUSH_INDEX(br);
    PUSH_INDEX(tr);
}

void batch_renderer_push_textured_quad(Batch_Renderer *r, f32 x, f32 y, f32 w, f32 h, f32 u, f32 v, GLuint texture) {
    assert(0);
}

#undef PUSH_VERTEX
#undef PUSH_INDEX