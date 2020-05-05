#version 440 core

layout(location = 1) uniform sampler2D u_textures[8];

in vec4 pass_color;
in vec2 pass_uv;
flat in float pass_tex;

out vec4 color;

void main() {
    color = texture(u_textures[int(pass_tex)], pass_uv) * pass_color;
}