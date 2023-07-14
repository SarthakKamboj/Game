#include "resources.h"
#include "stb/stb_image.h"
#include <iostream>
#include "utils/io.h"

// TEXTURE
texture_t create_texture(const char* path) {
	texture_t texture;

	int num_channels, width, height;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path, &width, &height, &num_channels, 0);

	glGenTextures(1, &texture.id);
	glBindTexture(GL_TEXTURE_2D, texture.id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	if (num_channels == 3) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);

	return texture;
}

void bind_texture(const texture_t& texture) {
	glBindTexture(GL_TEXTURE_2D, texture.id);
}

void unbind_texture() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

// SHADER
shader_t create_shader(const char* vert_path, const char* frag_path) {
	shader_t shader;

	GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

	int success;
	char info_log[512];

	std::string vert_code_str = io::get_file_contents(vert_path);
	const char* vert_shader_source = vert_code_str.c_str();
	glShaderSource(vert_shader, 1, &vert_shader_source, NULL);
	glCompileShader(vert_shader);
	glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vert_shader, 512, NULL, info_log);
		throw std::runtime_error("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" + std::string(info_log));
	}

	std::string frag_code_str = io::get_file_contents(frag_path);
	const char* frag_shader_source = frag_code_str.c_str();
	glShaderSource(frag_shader, 1, &frag_shader_source, NULL);
	glCompileShader(frag_shader);
	glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(frag_shader, 512, NULL, info_log);
		throw std::runtime_error("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" + std::string(info_log));
	}

	shader.id = glCreateProgram();
	glAttachShader(shader.id, vert_shader);
	glAttachShader(shader.id, frag_shader);
	glLinkProgram(shader.id);
	glGetProgramiv(shader.id, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader.id, 512, NULL, info_log);
		throw std::runtime_error("ERROR::SHADER::PROGRAM::LINKING_FAILED\n" + std::string(info_log));
	}

	return shader;
}

void bind_shader(const shader_t& shader) {
	glUseProgram(shader.id);
}

void unbind_shader() {
	glUseProgram(0);
}

void shader_set_mat4(shader_t& shader, const char* var_name, const glm::mat4& mat) {
	glUseProgram(shader.id);
	unsigned int loc = glGetUniformLocation(shader.id, var_name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

void shader_set_int(shader_t& shader, const char* var_name, const int val) {
	glUseProgram(shader.id);
	unsigned int loc = glGetUniformLocation(shader.id, var_name);
	glUniform1i(loc, val);
}

void shader_set_vec3(shader_t& shader, const char* var_name, const glm::vec3& v) {
	glUseProgram(shader.id);
	unsigned int loc = glGetUniformLocation(shader.id, var_name);
	glUniform3fv(loc, 1, glm::value_ptr(v));
}

glm::vec3 shader_get_vec3(const shader_t& shader, const char* var_name) {
	glm::vec3 v;
	unsigned int loc = glGetUniformLocation(shader.id, var_name);
	glGetUniformfv(shader.id, loc, &v[0]);
	return v;
}
