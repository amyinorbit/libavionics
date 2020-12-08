#version 400 core
uniform sampler2D tex;

in vec2 uv;
out vec4 out_fragColor;

void main() {
    out_fragColor = texture(tex, uv);
}
