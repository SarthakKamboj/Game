#version 410 core

out vec4 frag_color;

in vec2 tex_coord;

uniform sampler2D char;
uniform vec3 color;

void main() {
    float c = texture(char, tex_coord).r;
	frag_color = vec4(color, 1.0) * vec4(1,1,1,c);
}