template<GLenum type>
struct GL_Buffer {
    GLuint id;

    void init(u64 size, GLenum usage) {
        glGenBuffers(1, &id);

        bind();
        glBufferData(type, size, nullptr, usage);
        unbind();
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
        bind();
        glBufferSubData(type, 0, size, data);
        unbind();
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
            default:
                assert(0);
                break;
        }
    }
};

struct Vertex_Array {
    GLuint id;

    void init() {
        glGenVertexArrays(1, &id);
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
        bind();
        vbo.bind();

        s64 size = 0;
        for(auto& e: format) {
            size += e.size();
        }

        s32 i = 0;
        s64 offset = 0;
        for(auto& e: format) {
            glEnableVertexAttribArray(i);

            switch(e.type) {
                case GL_FLOAT:
                    glVertexAttribPointer(i, e.count, e.type, GL_FALSE, size, (void*) offset);
                    break;
                case GL_INT:
                    glVertexAttribIPointer(i, e.count, e.type, size, (void*) offset);
                    break;
                default:
                    assert(0);
            }
            
            offset += e.size();
            i++;
        }

        unbind();
        vbo.unbind();
    }

    void set_index_buffer(Index_Buffer& ibo) {
        bind();
        ibo.bind();
        unbind();
        ibo.unbind();
    }
};