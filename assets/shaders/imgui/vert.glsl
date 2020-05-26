#version 440 core

layout(location = 0) in vec2 v_pos;
layout(location = 1) in vec2 v_uv;
layout(location = 2) in vec4 v_color;

uniform mat4 u_proj;

smooth out vec2 pass_uv;
smooth out vec4 pass_color;

void main() {
	gl_Position = u_proj * vec4(v_pos.xy, 0, 1);
	pass_uv = v_uv;
	pass_color = v_color;
}