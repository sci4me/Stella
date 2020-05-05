#version 440 core

layout(location = 0) uniform mat4 u_proj;
// TODO: view matrix?

layout(location = 0) in vec2 v_pos;
layout(location = 1) in vec4 v_color;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in float v_tex;

out vec4 pass_color;
out vec2 pass_uv;
flat out float pass_tex;

void main() {
    pass_color = v_color;
    pass_uv = v_uv;
    pass_tex = v_tex;
    gl_Position = u_proj * vec4(v_pos.xy, 0.0, 1.0);
}