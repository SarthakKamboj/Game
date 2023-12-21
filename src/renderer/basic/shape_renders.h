#pragma once

#include "transform/transform.h"
#include "glm/glm.hpp"
#include "../opengl/object_data.h"

/// <summary>
/// Represents the render settings for a quad in the game. Stores
/// information about the transform its associated with, the texture its associated with
/// and its influence, its color, width, height, and if it's in wireframe mode.
/// </summary>
struct quad_render_t {
    int handle = -1;
	int transform_handle = -1;
    int tex_handle = -1;
    float tex_influence = 0;
    // internal transform, especially needed for setting width and height
	transform_t _internal_transform;
	glm::vec3 color = glm::vec3(0, 0, 0);
	float width = -1.f, height = -1.f;
	bool wireframe_mode = false;

    // each rectangle_rectangle_t is associated with one set of opengl vao, vbo, ebo
	static opengl_object_data obj_data;
};

/// <summary>
/// Creates a quad to be render
/// </summary>
/// <param name="transform_handle">Transform handle associated with this render</param>
/// <param name="color">The base color</param>
/// <param name="width">In pixels</param>
/// <param name="height">In pixels</param>
/// <param name="wireframe">Whether wireframe mode or not, useful for debugging</param>
/// <param name="tex_influence">Influence of the texture if it exists</param>
/// <param name="tex_handle">Handle of the texture associated with the render quad</param>
/// <returns>The quad to be rendered</returns>
int create_quad_render(int transform_handle, glm::vec3& color, float width, float height, bool wireframe, float tex_influence, int tex_handle);

void set_quad_texture(int quad_handle, int tex_handle);

/// <summary>
/// Draw a particular quad
/// </summary>
/// <param name="quad">Quad/recntagle to draw</param>
void draw_quad_render(const quad_render_t& quad);

void add_debug_pt(glm::vec3& pt);
void draw_debug_pt(glm::vec3 pos);
void clear_debug_pts();

struct application_t;

/// <summary>
/// Draw all quads in the game
/// </summary>
void draw_quad_renders(application_t& app);

/**
 * @brief Delete a quad render
 * @param quad_handle 
*/
void delete_quad_render(int quad_handle);

struct font_char_t {
	glm::vec2 bearing = glm::vec2(0);
	float width = 0;
	float height = 0;
	char c = 0;
	int texture_handle = -1;
	float advance = -1;

	static opengl_object_data ui_opengl_data;
};
void init_fonts();

struct text_dim_t {
	float width = 0;
	float height = 0;
	float height_below_baseline = 0;
};
text_dim_t get_text_dimensions(const char* text);
void draw_text(const char* text, glm::vec2 starting_pos);