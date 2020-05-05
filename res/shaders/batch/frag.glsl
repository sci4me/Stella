#version 440 core

layout(location = 1) uniform sampler2D textures[8];

in vec4 pass_color;
in vec2 pass_uv;
flat in int pass_tex;

out vec4 color;

void main() {
    color = texture(textures[pass_tex], pass_uv) * pass_color;
}