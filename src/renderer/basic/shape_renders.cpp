#include "shape_renders.h"
#include "utils/conversion.h"
#include "../renderer.h"
#include "transform/transform.h"
#include <vector>

opengl_object_data rectangle_render_t::obj_data{};

static std::vector<rectangle_render_t> rectangles;

int create_rectangle_render(int transform_handle, glm::vec3& color, float width, float height, bool wireframe, float tex_influence, int tex_handle) {
    static int running_count = 0;
	rectangle_render_t rectangle;
	rectangle.transform_handle = transform_handle;
	rectangle.width = width;
    rectangle.tex_influence = tex_influence;
    rectangle.tex_handle = tex_handle;
	rectangle.height = height;
	rectangle.color = color;
	rectangle.wireframe_mode = wireframe;
    // we scale internally because the original rectangle is 1x1 pixel wide and high
	rectangle._internal_transform.scale = glm::vec3(width, height, 1.f);
    rectangle.handle = running_count;
    rectangles.push_back(rectangle);
    running_count++;
	return rectangle.handle;
}

void draw_rectangle_renders() {
    for (int i = 0; i < rectangles.size(); i++) {
        if (i == 96) {
            int j = 10;
        }
        const rectangle_render_t& rectangle = rectangles[i];
		draw_rectangle_render(rectangle);
	}
}

void draw_rectangle_render(const rectangle_render_t& rectangle) {
    // get the transform for that rectangle render
    transform_t* transform_ptr = get_transform(rectangle.transform_handle);
    assert(transform_ptr != NULL);
	transform_t cur_transform = *transform_ptr;
    
    // add the internal offset, especially necessary for scale to make sure width and height are abided by
	cur_transform.position += rectangle._internal_transform.position;
	cur_transform.rotation_deg += rectangle._internal_transform.rotation_deg;
	cur_transform.scale *= rectangle._internal_transform.scale;

    // get model matrix and color and set them in the shader
	glm::mat4 model_matrix = get_model_matrix(cur_transform);
    bind_texture(rectangle.tex_handle);
	shader_set_mat4(rectangle_render_t::obj_data.shader, "model", model_matrix);
	shader_set_vec3(rectangle_render_t::obj_data.shader, "color", rectangle.color);
	shader_set_float(rectangle_render_t::obj_data.shader, "tex_influence", rectangle.tex_influence);
	if (rectangle.wireframe_mode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
    // draw the rectangle render after setting all shader parameters
	draw_obj(rectangle_render_t::obj_data);
}
