#pragma once

#include "glad/glad.h"

/// <summary>
/// An OpenGL EBO
/// </summary>
struct ebo_t {
	GLuint id = 0;
	int num_indicies = -1;
};

/// <summary>
/// Creates an OpenGL EBO that will be mainly read-only after initial write
/// </summary>
/// <param name="indicies">An array of indicies</param>
/// <param name="size_of_buffer">The size of indicies in bytes (should be a multiple of the size of unsigned
/// int on the system, which is most likely 4 bytes)</param>
/// <returns></returns>
ebo_t create_ebo(const unsigned int* indicies, const int size_of_buffer);

/// <summary>
/// Creates an OpenGL EBO where the usage can be set manually
/// </summary>
/// <param name="indicies">An array of indicies</param>
/// <param name="size_of_buffer">The size of indicies in bytes (should be a multiple of the size of unsigned
/// int on the system, which is most likely 4 bytes)</param>
/// <param name="usage_pattern">The usage pattern of the EBO. Particularly useful to declare when a EBO
/// will be written to often.</param>
/// <returns></returns>
ebo_t create_ebo(const unsigned int* indicies, const int size_of_buffer, GLenum usage_pattern);

/// <summary>
/// Draw a particular EBO. Note, the EBO must be binded at this point for this function to have effect.
/// </summary>
/// <param name="ebo">EBO to draw</param>
void draw_ebo(const ebo_t& ebo);

/// <summary>
/// Bind an EBO for drawing or other forms of manipulation
/// </summary>
/// <param name="ebo">EBO to bind</param>
void bind_ebo(const ebo_t& ebo);

/// <summary>
/// Unbind an EBO to ensure changes aren't made to it anymore or finished drawing
/// </summary>
void unbind_ebo();

/// <summary>
/// Delete an EBO
/// </summary>
/// <param name="ebo">EBO to delete</param>
void delete_ebo(const ebo_t& ebo);

/// <summary>
/// OpenGL Vertex Buffer Object
/// </summary>
struct vbo_t {
	GLuint id = 0;
};

/// <summary>
/// Creates an OpenGL VBO
/// </summary>
/// <param name="vertices">An array of the vertices</param>
/// <param name="data_size"></param>
/// <returns></returns>
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
