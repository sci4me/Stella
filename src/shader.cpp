void program_attach_shader(GLuint program, char *name, GLenum type) {
    Entire_File source = read_entire_file(name);

    if(!source.data) {
        tfprintf(STDERR, "Unable to read file: %s\n", name);
        mlc_exit(1);
    }

    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, (char const* const*) &source.data, 0);
    glCompileShader(id);

    source.deinit();

    GLint success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint info_log_length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_log_length);

        GLchar *info_log = (GLchar*) talloc(sizeof(GLchar) * (info_log_length + 1));
        assert(info_log); // NOTE: Yes I'm being lazy, fite me.
        glGetShaderInfoLog(id, info_log_length, 0, info_log);

        tfprintf(STDERR, "Error compiling shader `%s`:\n%s\n", name, info_log);
        mlc_exit(1);
    }

    glAttachShader(program, id);
}

enum {
    VERTEX_SHADER = 1,
    GEOMETRY_SHADER = 2,
    FRAGMENT_SHADER = 4
};

GLuint load_shader_program(const char *name, GLenum flags) {
    GLuint p = glCreateProgram();

    char buffer[1024];

    #define SHADER_TYPE(t, n, s)\
        if(flags & t) {\
            stbsp_sprintf(buffer, "assets/shaders/%s/%s.glsl", name, n);\
            program_attach_shader(p, buffer, s);\
        }

    SHADER_TYPE(VERTEX_SHADER, "vert", GL_VERTEX_SHADER)
    SHADER_TYPE(GEOMETRY_SHADER, "geom", GL_GEOMETRY_SHADER)
    SHADER_TYPE(FRAGMENT_SHADER, "frag", GL_FRAGMENT_SHADER)

    #undef SHADER_TYPE

    GLint success;
    glLinkProgram(p);
    glGetProgramiv(p, GL_LINK_STATUS, &success);
    if (!success) {
        GLint info_log_length;
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &info_log_length);

        // TODO: maybe _don't_ use alloca here lol
        GLchar *info_log = (GLchar*) talloc(sizeof(GLchar) * (info_log_length + 1));
        assert(info_log); // NOTE: Yes I'm being lazy, fite me.
        glGetProgramInfoLog(p, info_log_length, 0, info_log);

        tfprintf(STDERR, "%s\n", info_log);
        mlc_exit(1);
    }

    return p;
}