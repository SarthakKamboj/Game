#pragma once

#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include <string>

/// <summary>
/// OpenGL Shader
/// </summary>
struct shader_t {
	GLuint id = 0;
};

/// <summary>
/// Create an OpenGL shader given the vertex and fragment shader
/// </summary>
/// <param name="vert_path">The path to the GLSL vertex shader</param>
/// <param name="frag_path">The path to the GLSL fragment shader</param>
/// <returns>The shader</returns>
shader_t create_shader(const char* vert_path, const char* frag_path);

/// <summary>
/// Bind the shader by object
/// </summary>
/// <param name="shader"></param>
void bind_shader(const shader_t& shader);

/// <summary>
/// Unbind currently bound shader
/// </summary>
void unbind_shader();

/// <summary>
/// Set the 4x4 matrix in the shader by name
/// </summary>
/// <param name="shader"></param>
/// <param name="var_name">Name of the 4x4 matrix variable</param>
/// <param name="mat">4x4 matrix data</param>
void shader_set_mat4(shader_t& shader, const char* var_name, const glm::mat4& mat);

/// <summary>
/// Set int in shader by name
/// </summary>
/// <param name="shader"></param>
/// <param name="var_name"></param>
/// <param name="val">Int val</param>
void shader_set_int(shader_t& shader, const char* var_name, const int val);

/// <summary>
/// Set vec3 in shader by name
/// </summary>
/// <param name="shader"></param>
/// <param name="var_name"></param>
/// <param name="v">Vector3 data</param>
void shader_set_vec3(shader_t& shader, const char* var_name, const glm::vec3& v);

/// <summary>
/// Get the vector3 for a particular shader
/// </summary>
/// <param name="shader"></param>
/// <param name="var_name">Name of the Vector3 variable</param>
/// <returns></returns>
glm::vec3 shader_get_vec3(const shader_t& shader, const char* var_name);

/// <summary>
/// Set a float in a shader
/// </summary>
/// <param name="shader"></param>
/// <param name="var_name">Name of float variable</param>
/// <param name="val">Float value</param>
void shader_set_float(shader_t& shader, const char* var_name, const float val);

/// <summary>
/// Stores a OpenGL Texture, the slot it is associated with, and its file path
/// </summary>
struct texture_t {
	GLuint id;
    int handle = -1;
    char path[1024];
    unsigned tex_slot = 0;
};

/// <summary>
/// Create a texture
/// </summary>
/// <param name="path">File path of the texture</param>
/// <param name="tex_slot">Its texture slot</param>
/// <returns>The handle of the texture</returns>
int create_texture(const char* path, int tex_slot);

/// <summary>
/// Get the texture given the handle
/// </summary>
/// <param name="handle">Texture handle</param>
/// <returns>A pointer to the texture, can be NULL be doesn't exist</returns>
texture_t* get_tex(int handle);

/// <summary>
/// Bind a texture
/// </summary>
/// <param name="texture">Texture to bind</param>
void bind_texture(const texture_t& texture);

/// <summary>
/// Bind a texture
/// </summary>
/// <param name="handle">Handle of texture to bind</param>
void bind_texture(int handle);

/// <summary>
/// Unbind the texture on what texture slot is currently active
/// </summary>
void unbind_texture();
