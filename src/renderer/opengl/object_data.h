#pragma once

#include "buffers.h"
#include "resources.h"

struct opengl_object_data {
	vao_t vao;
	ebo_t ebo;
	shader_t shader;
	vbo_t vbo;
};

void draw_obj(const opengl_object_data& data);
