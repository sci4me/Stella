#version 440 core

layout(location = 0) uniform mat4 u_proj;
layout(location = 1) uniform vec4 u_color;

layout(location = 0) in vec2 v_pos;

out vec4 pass_color;

void main() {
	gl_Position = u_proj * vec4(v_pos.xy, 0.0, 1.0);
	pass_color = u_color;
}