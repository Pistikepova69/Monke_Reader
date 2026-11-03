#version 440
in vec2 vUV;

uniform sampler2D tex;

out vec4 Frag_Color;

void main() {
    Frag_Color = texture(tex, vUV);
}