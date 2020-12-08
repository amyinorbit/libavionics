#version 400 core

layout (location=0) in vec2 in_pos;

uniform mat4 proj;
uniform vec2 size;
uniform vec2 pos;

out vec2 uv;

void main() {
    vec2 total_pos = pos + (in_pos * size);
    uv = in_pos;
    gl_Position = proj * vec4(total_pos, 0.f, 1.f);
}
