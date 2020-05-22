template<GLenum type>
struct GL_Buffer {
    GLuint id;

    void init(u64 size, GLenum usage) {
        glCreateBuffers(1, &id);
        glNamedBufferData(id, size, nullptr, usage);
    }

    void deinit() {
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

    Vertex_Element(GLenum _type, s32 _count) : type(_type), count(_count) {}

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

    void deinit() {
        glDeleteVertexArrays(1, &id);
    }

    void bind() {
        glBindVertexArray(id);
    }

    void unbind() {
        glBindVertexArray(0);
    }

    template<typename... T>
    void add_vertex_buffer(Vertex_Buffer& vbo, T... _format) {
        static_assert((otr::type_eq<T, Vertex_Element> && ...));
        
        auto n = sizeof...(_format);
        Vertex_Element format[] = { _format... };

        s64 stride = 0;
        for(u32 i = 0; i < n; i++) {
            stride += format[i].size();
        }

        glVertexArrayVertexBuffer(id, binding_index, vbo.id, 0, stride);

        s64 offset = 0;
        for(u32 i = 0; i < n; i++) {
            glEnableVertexArrayAttrib(id, i);
            glVertexArrayAttribBinding(id, i, binding_index);

            auto const& e = format[i];
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
        }

        binding_index++;
    }

    void set_index_buffer(Index_Buffer& ibo) {
        glVertexArrayElementBuffer(id, ibo.id);
    }
};