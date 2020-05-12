template<GLenum type>
struct GL_Buffer {
    GLuint id;

    void init(u64 size, GLenum usage) {
        glCreateBuffers(1, &id);
        glNamedBufferData(id, size, nullptr, usage);
    }

    void free() {
        glDeleteBuffers(1, &id);
    }

    void bind() {
        glBindBuffer(type, id);
    }

    void unbind() {
        glBindBuffer(type, 0);
    }

    void set_data(void *data, u32 size) {
        glNamedBufferSubData(id, 0, size, data);
    }
};

using Vertex_Buffer = GL_Buffer<GL_ARRAY_BUFFER>;
using Index_Buffer = GL_Buffer<GL_ELEMENT_ARRAY_BUFFER>;

struct Vertex_Element {
    GLenum type;
    s32 count;

    s64 size() const {
        switch(type) {
            case GL_FLOAT:
            case GL_INT:
                return count * 4;
        }
        
        assert(0);
        return 0;
    }
};

struct Vertex_Array {
    GLuint id;
    GLuint binding_index;

    void init() {
        glCreateVertexArrays(1, &id);
        binding_index = 0;
    }

    void free() {
        glDeleteVertexArrays(1, &id);
    }

    void bind() {
        glBindVertexArray(id);
    }

    void unbind() {
        glBindVertexArray(0);
    }

    void add_vertex_buffer(Vertex_Buffer& vbo, std::initializer_list<Vertex_Element> format) {
        s64 stride = 0;
        for(auto& e: format) {
            stride += e.size();
        }

        glVertexArrayVertexBuffer(id, binding_index, vbo.id, 0, stride);

        s32 i = 0;
        s64 offset = 0;
        for(auto& e: format) {
            glEnableVertexArrayAttrib(id, i);
            glVertexArrayAttribBinding(id, i, binding_index);

            switch(e.type) {
                case GL_FLOAT:
                    glVertexArrayAttribFormat(id, i, e.count, e.type, GL_FALSE, offset);
                    break;
                case GL_INT:
                    glVertexArrayAttribIFormat(id, i, e.count, e.type, offset);
                    break;
                default:
                    assert(0);
            }
            
            offset += e.size();
            i++;
        }

        binding_index++;
    }

    void set_index_buffer(Index_Buffer& ibo) {
        glVertexArrayElementBuffer(id, ibo.id);
    }
};