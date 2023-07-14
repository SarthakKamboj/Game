#version 410 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_color;

uniform mat4 model;
uniform mat4 projection;

out vec4 out_color;

void main() {
	gl_Position = projection * model * vec4(in_pos, 1.0);
	// gl_Position = model * vec4(in_pos, 1.0);
	out_color = vec4(in_color, 1.0);
}