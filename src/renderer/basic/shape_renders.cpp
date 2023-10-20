#include "shape_renders.h"
#include "utils/conversion.h"
#include "../renderer.h"
#include "transform/transform.h"
#include <vector>

opengl_object_data quad_render_t::obj_data{};

static std::vector<quad_render_t> quads;

int create_quad_render(int transform_handle, glm::vec3& color, float width, float height, bool wireframe, float tex_influence, int tex_handle) {
    static int running_count = 0;
	quad_render_t quad;
	quad.transform_handle = transform_handle;
	quad.width = width;
    quad.tex_influence = tex_influence;
    quad.tex_handle = tex_handle;
	quad.height = height;
	quad.color = color;
	quad.wireframe_mode = wireframe;
    // we scale internally because the original rectangle is 1x1 pixel wide and high
	quad._internal_transform.scale = glm::vec3(width, height, 1.f);
    quad.handle = running_count;
    quads.push_back(quad);
    running_count++;
	return quad.handle;
}

void draw_quad_renders() {
    for (int i = 0; i < quads.size(); i++) {
        const quad_render_t& quad = quads[i];
		draw_quad_render(quad);
	}
}

void draw_quad_render(const quad_render_t& quad) {
    // get the transform for that rectangle render
    transform_t* transform_ptr = get_transform(quad.transform_handle);
    assert(transform_ptr != NULL);
	transform_t cur_transform = *transform_ptr;
    
    // add the internal offset, especially necessary for scale to make sure width and height are abided by
	cur_transform.position += quad._internal_transform.position;
	cur_transform.rotation_deg += quad._internal_transform.rotation_deg;
	cur_transform.scale *= quad._internal_transform.scale;

    // get model matrix and color and set them in the shader
	glm::mat4 model_matrix = get_model_matrix(cur_transform);
    bind_texture(quad.tex_handle);
	shader_set_mat4(quad_render_t::obj_data.shader, "model", model_matrix);
	shader_set_vec3(quad_render_t::obj_data.shader, "color", quad.color);
	shader_set_float(quad_render_t::obj_data.shader, "tex_influence", quad.tex_influence);
	if (quad.wireframe_mode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
    // draw the rectangle render after setting all shader parameters
	draw_obj(quad_render_t::obj_data);
}
