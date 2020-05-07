#version 440 core

uniform sampler2D u_textures[16];

in vec4 pass_color;
in vec2 pass_uv;
flat in int pass_tex;

layout(location = 0) out vec4 color;

void main() {
    color = texture(u_textures[pass_tex], pass_uv) * pass_color;
}