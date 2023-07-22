#pragma once

#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include <string>

struct shader_t {
	GLuint id = 0;
};

shader_t create_shader(const char* vert_path, const char* frag_path);
void bind_shader(const shader_t& shader);
void unbind_shader();
void shader_set_mat4(shader_t& shader, const char* var_name, const glm::mat4& mat);
void shader_set_int(shader_t& shader, const char* var_name, const int val);
void shader_set_vec3(shader_t& shader, const char* var_name, const glm::vec3& v);
glm::vec3 shader_get_vec3(const shader_t& shader, const char* var_name);
void shader_set_float(shader_t& shader, const char* var_name, const float val);

struct texture_t {
	GLuint id;
    int handle = -1;
    char path[1024];
    unsigned tex_slot = 0;
};

int create_texture(const char* path, int tex_slot);
texture_t* get_tex(int handle);
void bind_texture(const texture_t& texture);
void bind_texture(int handle);
void unbind_texture();
