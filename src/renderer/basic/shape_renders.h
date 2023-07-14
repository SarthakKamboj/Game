#pragma once

#include "transform/transform.h"
#include "glm/glm.hpp"
#include "../opengl/object_data.h"

struct rectangle_render_t {
    int handle = -1;
	int transform_handle = -1;
    // internal transform, especially needed for setting width and height
	transform_t _internal_transform;
	glm::vec3 color = glm::vec3(0, 0, 0);
	float width = -1.f, height = -1.f;
	bool wireframe_mode = false;

    // each rectangle_rectangle_t is associated with one set of opengl vao, vbo, ebo
	static opengl_object_data obj_data;
};

int create_rectangle_render(int transform_handle, glm::vec3& color, float width, float height, bool wireframe);
void draw_rectangle_render(const rectangle_render_t& rectangle);
void draw_rectangle_renders();
