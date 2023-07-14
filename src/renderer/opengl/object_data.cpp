#include "object_data.h"
#include "glad/glad.h"

void draw_obj(const opengl_object_data& data) {
	bind_shader(data.shader);
	bind_vao(data.vao);
	draw_ebo(data.ebo);
}
