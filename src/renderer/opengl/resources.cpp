#include "resources.h"
#include "stb/stb_image.h"
#include <iostream>
#include "utils/io.h"
#include <vector>
#include "glad/glad.h"
#include "constants.h"

#include "buffers.h"

static std::vector<texture_t> textures;
static int tex_running_cnt = 0;

// TEXTURE
int create_texture(const char* path, int tex_slot) {
    for (int i = 0; i < textures.size(); i++) {
        if (strcmp(path, textures[i].path) == 0) {
            return textures[i].handle;
        }
    }

	texture_t texture{};
    texture.tex_slot = tex_slot;
	int num_channels, width, height;
	stbi_set_flip_vertically_on_load(true);

	unsigned char* data = stbi_load(path, &width, &height, &num_channels, 0);
	game_assert(data);

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
    texture.handle = tex_running_cnt;
    tex_running_cnt++;

    textures.push_back(texture);

	game_assert(!detect_gl_error());

	return texture.handle;
}

int create_texture(unsigned char* buffer, int width, int height, int tex_slot) {
	texture_t texture;
	texture.handle = tex_running_cnt;
	tex_running_cnt++;

	glGenTextures(1, &texture.id);
	glBindTexture(GL_TEXTURE_2D, texture.id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, (void*)buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	texture.tex_slot = tex_slot;

	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		// Log or print the OpenGL error
		std::cout << "ran into opengl error" << std::endl;
	}

	textures.push_back(texture);

	return texture.handle;
}

void bind_texture(const texture_t& texture) {
    glActiveTexture(GL_TEXTURE0 + texture.tex_slot);
	glBindTexture(GL_TEXTURE_2D, texture.id);
}

void bind_texture(int handle, bool required_bind) {
    for (int i = 0; i < textures.size(); i++) {
        if (textures[i].handle == handle) {
            glActiveTexture(GL_TEXTURE0 + textures[i].tex_slot);
	        glBindTexture(GL_TEXTURE_2D, textures[i].id);
            return;
        }
    }

	if (required_bind) {
		// game_assert("could not find texture even though required bind was asserted");
		game_assert(false);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

void unbind_texture() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

texture_t* get_tex(int handle) {
    for (int i = 0; i < textures.size(); i++) {
        if (textures[i].handle == handle) {
            return &textures[i];
        }
    }
    return NULL;
}

// SHADER
shader_t create_shader(const char* vert_file, const char* frag_file) {
	shader_t shader;

	GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

	int success;
	char info_log[512];

	char resource_path[256]{};
	io::get_resources_folder_path(resource_path);

	char vert_path[256]{};
	sprintf(vert_path, "%s\\%s\\%s", resource_path, SHADERS_FOLDER, vert_file);
	char frag_path[256]{};
	sprintf(frag_path, "%s\\%s\\%s", resource_path, SHADERS_FOLDER, frag_file);

	std::cout << vert_path << std::endl;
	std::cout << frag_path << std::endl;

	std::string vert_code_str = io::get_file_contents(vert_path);
	const char* vert_shader_source = vert_code_str.c_str();
	glShaderSource(vert_shader, 1, &vert_shader_source, NULL);
	glCompileShader(vert_shader);
	glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vert_shader, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" + std::string(info_log) << std::endl;
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
		std::cout << "error::shader::fragment::compilation_failed\n" + std::string(info_log) << std::endl;
		throw std::runtime_error("error::shader::fragment::compilation_failed\n" + std::string(info_log));
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
	GLint loc = glGetUniformLocation(shader.id, var_name);
    if (loc == -1) {
        std::cout << var_name << " does not exist in shader " << shader.id << std::endl;
    }
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

void shader_set_int(shader_t& shader, const char* var_name, const int val) {
	glUseProgram(shader.id);
	GLint loc = glGetUniformLocation(shader.id, var_name);
    if (loc == -1) {
        std::cout << var_name << " does not exist in shader " << shader.id << std::endl;
    }
	glUniform1i(loc, val);
}

void shader_set_vec3(shader_t& shader, const char* var_name, const glm::vec3& v) {
	glUseProgram(shader.id);
	GLint loc = glGetUniformLocation(shader.id, var_name);
    if (loc == -1) {
        std::cout << var_name << " does not exist in shader " << shader.id << std::endl;
    }
	glUniform3fv(loc, 1, glm::value_ptr(v));
}

void shader_set_float(shader_t& shader, const char* var_name, const float val) {
	glUseProgram(shader.id);
	GLint loc = glGetUniformLocation(shader.id, var_name);
    if (loc == -1) {
        std::cout << var_name << " does not exist in shader " << shader.id << std::endl;
    }
	glUniform1f(loc, val);
}

glm::vec3 shader_get_vec3(const shader_t& shader, const char* var_name) {
	glm::vec3 v;
	GLint loc = glGetUniformLocation(shader.id, var_name);
    if (loc == -1) {
        std::cout << var_name << " does not exist in shader " << shader.id << std::endl;
    }
	glGetUniformfv(shader.id, loc, &v[0]);
	return v;
}
