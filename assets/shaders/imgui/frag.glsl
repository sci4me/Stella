#version 440 core

uniform sampler2D u_texture;

smooth in vec2 pass_uv;
smooth in vec4 pass_color;

layout(location = 0) out vec4 color;

void main() {
	color = pass_color * texture(u_texture, pass_uv.st);
}