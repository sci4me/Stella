#version 440 core

uniform sampler2D u_textures[16];

smooth in vec2 pass_uv;
flat in int pass_tex;
flat in float pass_uv_rotation;

layout(location = 0) out vec4 color;

vec2 rotateUV(vec2 uv, float rotation) {
    float mid = 0.5;
    return vec2(
        cos(rotation) * (uv.x - mid) + sin(rotation) * (uv.y - mid) + mid,
        cos(rotation) * (uv.y - mid) - sin(rotation) * (uv.x - mid) + mid
    );
}

void main() {
    vec2 rotated_uv = rotateUV(pass_uv, pass_uv_rotation);
    color = texture(u_textures[pass_tex], rotated_uv);
}