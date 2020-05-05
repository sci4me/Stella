#version 440 core

const vec2 vertices[4] = {
    vec2( 1.0, -1.0), 
    vec2( 1.0,  1.0), 
    vec2(-1.0, -1.0), 
    vec2(-1.0,  1.0)
};

const vec2 uvs[4] = {
    vec2(1.0f, 1.0f),
    vec2(1.0f, 0.0f),
    vec2(0.0f, 1.0f),
    vec2(0.0f, 0.0f)
};

layout(location = 0) in float dummy;

out vec2 pass_uv;

void main() {
    gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
    pass_uv = uvs[gl_VertexID];
}