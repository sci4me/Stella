#version 440 core

layout(location = 0) uniform sampler2D u_scene;

in vec2 pass_uv;

layout(location = 0) out vec4 color;

//const vec3 ambient_light = vec3(0.3, 0.3, 0.3);

const vec2 light = { 400.0, 300.0 };
const vec3 light_color = { 1.0, 1.0, 1.0 };
const float light_radius = 20.0;
const float light_intensity = 10;

void main() {
    vec2 pixel = gl_FragCoord.xy;
    vec2 to_pixel = light - pixel;
    float dist = length(to_pixel);
    
    float atten = 1.0 / dist * light_radius;
    vec4 c = vec4(atten, atten, atten, 1.0) * vec4(light_color, 1.0);

    color = texture(u_scene, pass_uv) * c;
}