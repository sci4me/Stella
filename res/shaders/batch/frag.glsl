#version 440 core

layout(location = 2) uniform sampler2D u_textures[8];

in vec4 pass_color;
in vec2 pass_uv;
flat in int pass_tex;

out vec4 color;

void main() {
    color = texture(u_textures[pass_tex], pass_uv) * pass_color;
}