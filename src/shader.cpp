void program_attach_shader(GLuint program, char *name, GLenum type) {
    Buffer source = read_entire_file(name);

    if(!source.data) {
        tfprintf(STDERR, "Unable to read file: %s\n", name);
        mlc_exit(1);
    }

    GLuint id = gl.CreateShader(type);
    gl.ShaderSource(id, 1, (char const* const*) &source.data, 0);
    gl.CompileShader(id);

    mlc_free(source.data);

    GLint success;
    gl.GetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint info_log_length;
        gl.GetShaderiv(id, GL_INFO_LOG_LENGTH, &info_log_length);

        GLchar *info_log = (GLchar*) talloc(sizeof(GLchar) * (info_log_length + 1));
        assert(info_log); // NOTE: Yes I'm being lazy, fite me.
        gl.GetShaderInfoLog(id, info_log_length, 0, info_log);

        tfprintf(STDERR, "Error compiling shader `%s`:\n%s\n", name, info_log);
        mlc_exit(1);
    }

    gl.AttachShader(program, id);
}

enum {
    VERTEX_SHADER = 1,
    GEOMETRY_SHADER = 2,
    FRAGMENT_SHADER = 4
};

GLuint load_shader_program(const char *name, GLenum flags) {
    GLuint p = gl.CreateProgram();

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
    gl.LinkProgram(p);
    gl.GetProgramiv(p, GL_LINK_STATUS, &success);
    if (!success) {
        GLint info_log_length;
        gl.GetProgramiv(p, GL_INFO_LOG_LENGTH, &info_log_length);

        GLchar *info_log = (GLchar*) talloc(sizeof(GLchar) * (info_log_length + 1));
        assert(info_log); // NOTE: Yes I'm being lazy, fite me.
        gl.GetProgramInfoLog(p, info_log_length, 0, info_log);

        tfprintf(STDERR, "%s\n", info_log);
        mlc_exit(1);
    }

    return p;
}