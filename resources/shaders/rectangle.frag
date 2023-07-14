#version 410 core

out vec4 frag_color;
in vec4 out_color;

uniform vec3 rec_color;

void main() {
	frag_color = vec4(rec_color, 1.0);
}