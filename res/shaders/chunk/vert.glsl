#version 440 core

uniform mat4 u_proj;
uniform mat4 u_view;

layout(location = 0) in vec2 v_pos;
layout(location = 1) in vec2 v_uv;
layout(location = 2) in int v_tex;

smooth out vec2 pass_uv;
flat out int pass_tex;

void main() {
    gl_Position = u_proj * u_view * vec4(v_pos.xy, 0.0, 1.0);
    pass_uv = v_uv;
    pass_tex = v_tex;
}