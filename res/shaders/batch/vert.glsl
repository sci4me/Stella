#version 440 core

layout(location = 0) uniform mat4 u_proj;
layout(location = 1) uniform float u_scale;
// TODO: view matrix?

layout(location = 0) in vec2 v_pos;
layout(location = 1) in vec4 v_color;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in int v_tex;

out vec4 pass_color;
out vec2 pass_uv;
flat out int pass_tex;

void main() {
    gl_Position = u_proj * vec4(v_pos.xy * u_scale, 0.0, 1.0);
    pass_color = v_color;
    pass_uv = v_uv;
    pass_tex = v_tex;
}