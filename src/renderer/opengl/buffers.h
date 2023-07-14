#pragma once

#include "glad/glad.h"

struct ebo_t {
	GLuint id = 0;
	int num_indicies = -1;
};

ebo_t create_ebo(const unsigned int* indicies, const int size_of_buffer);
ebo_t create_ebo(const unsigned int* indicies, const int size_of_buffer, GLenum usage_pattern);
void draw_ebo(const ebo_t& ebo);
void bind_ebo(const ebo_t& ebo);
void unbind_ebo();
void delete_ebo(const ebo_t& ebo);

struct vbo_t {
	GLuint id = 0;
};

vbo_t create_vbo(const float* vertices, const int data_size);
void bind_vbo(const vbo_t& vbo);
void update_vbo_data(const vbo_t& vbo, const float* vertices, const int data_size);
void unbind_vbo();
void delete_vbo(const vbo_t& vbo);

struct vao_t {
	GLuint id = 0;
};

vao_t create_vao();
void bind_vao(const vao_t& vao);
void unbind_vao();
void vao_enable_attribute(vao_t& vao, const vbo_t& vbo, const int attrId, const int numValues, const int dType, const int normalized, const int stride, const int offset);
void delete_vao(const vao_t& vao);
