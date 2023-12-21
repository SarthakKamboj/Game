#include "shape_renders.h"
#include "utils/conversion.h"
#include "../renderer.h"
#include "transform/transform.h"
#include <vector>
#include "constants.h"
#include "../opengl/vertex.h"

#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H  
#include "unordered_map"

opengl_object_data quad_render_t::obj_data{};
std::vector<glm::vec3> debug_pts;

static std::vector<quad_render_t> quads;

int create_quad_render(int transform_handle, glm::vec3& color, float width, float height, bool wireframe, float tex_influence, int tex_handle) {
    static int running_count = 0;
	quad_render_t quad;
	quad._internal_transform.scale = glm::vec3(width, height, 1.f);
	quad.transform_handle = transform_handle;
	quad.width = width;
    quad.tex_influence = tex_influence;
    quad.tex_handle = tex_handle;
	quad.height = height;
	quad.color = color;
	quad.wireframe_mode = wireframe;
    // we scale internally because the original rectangle is 1x1 pixel wide and high
    quad.handle = running_count;
    quads.push_back(quad);
    running_count++;
	return quad.handle;
}

void set_quad_texture(int quad_handle, int tex_handle) {
	for (int i = 0; i < quads.size(); i++) {
        quad_render_t& quad = quads[i];
		if (quad.handle == quad_handle) {
			quad.tex_handle = tex_handle;
			return;
		}
	}	
}

void draw_quad_renders(application_t& app) {
	glm::mat4 view_mat = app.camera.get_view_matrix();
	shader_set_mat4(quad_render_t::obj_data.shader, "view", view_mat);
    for (int i = 0; i < quads.size(); i++) {
        const quad_render_t& quad = quads[i];
		draw_quad_render(quad);
	}
	for (glm::vec3& dp : debug_pts) {
		draw_debug_pt(dp);
	}
}


void add_debug_pt(glm::vec3& pt) {
	debug_pts.push_back(pt);
}

void clear_debug_pts() {
	debug_pts.clear();
}

void draw_debug_pt(glm::vec3 pos) {
	transform_t cur_transform; 
	cur_transform.position = pos;
	cur_transform.scale *= 5.f;

	glm::mat4 model_matrix = get_model_matrix(cur_transform);
    // get model matrix and color and set them in the shader
	shader_set_mat4(quad_render_t::obj_data.shader, "model", model_matrix);
	shader_set_vec3(quad_render_t::obj_data.shader, "color", glm::vec3(1,0,0));
	shader_set_float(quad_render_t::obj_data.shader, "tex_influence", 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // draw the rectangle render after setting all shader parameters
	draw_obj(quad_render_t::obj_data);	
}

void draw_quad_render(const quad_render_t& quad) {
    // get the transform for that rectangle render
    transform_t* transform_ptr = get_transform(quad.transform_handle);
    assert(transform_ptr != NULL);
	transform_t cur_transform = *transform_ptr;
    
    // add the internal offset, especially necessary for scale to make sure width and height are abided by
#if 1
	cur_transform.position += quad._internal_transform.position;
	cur_transform.rotation_deg += quad._internal_transform.rotation_deg;
	cur_transform.scale *= quad._internal_transform.scale;
#endif

	glm::mat4 model_matrix = get_model_matrix(cur_transform);
    // get model matrix and color and set them in the shader
	shader_set_mat4(quad_render_t::obj_data.shader, "model", model_matrix);
	shader_set_vec3(quad_render_t::obj_data.shader, "color", quad.color);
#if _TESTING	
	shader_set_float(quad_render_t::obj_data.shader, "tex_influence", 0);
    unbind_texture();
#else
	shader_set_float(quad_render_t::obj_data.shader, "tex_influence", quad.tex_influence);
    bind_texture(quad.tex_handle);
#endif
	if (quad.wireframe_mode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
    // draw the rectangle render after setting all shader parameters
	draw_obj(quad_render_t::obj_data);
}

void delete_quad_render(int quad_handle) {
	int i_to_remove = -1;
	for (int i = 0; i < quads.size(); i++) {
		quad_render_t& quad = quads[i];
		if (quad.handle == quad_handle) {
			i_to_remove = i;
			break;
		}
	}
	if (i_to_remove != -1) {
		quads.erase(quads.begin() + i_to_remove);
	}
}


opengl_object_data font_char_t::ui_opengl_data{};
std::unordered_map<unsigned char, font_char_t> chars;

void init_fonts() {
	FT_Library lib;
	if (FT_Init_FreeType(&lib)) {
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    	return;
	}
	FT_Face face;
	if (FT_New_Face(lib, "C:\\Sarthak\\projects\\game\\resources\\fonts\\Courier_Prime\\CourierPrime-Regular.ttf", 0, &face))
	{
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;  
		return;
	}
	FT_Set_Pixel_Sizes(face, 0, 48);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for (unsigned char c = 0; c < 128; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			return;
		}

		font_char_t font_char;
		font_char.c = c;
		if (c == ' ') {
			font_char.width = 10;
			font_char.advance = 30;
			font_char.height = 20;
			font_char.bearing.x = 5;
			font_char.bearing.y = 5;
		} else {
			font_char.width = face->glyph->bitmap.width;
			font_char.advance = font_char.width * 1.025f;
			font_char.height = face->glyph->bitmap.rows;
			font_char.bearing.x = face->glyph->bitmap_left;
			font_char.bearing.y = face->glyph->bitmap_top;
		}

		font_char.texture_handle = create_texture(face->glyph->bitmap.buffer, font_char.width, font_char.height, 0);
		chars[c] = font_char;
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	
	opengl_object_data& data = font_char_t::ui_opengl_data;
	data.vao = create_vao();
	data.vbo = create_dyn_vbo(4 * sizeof(vertex_t));
	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};

    // set up ebo with indicies
	data.ebo = create_ebo(indices, sizeof(indices));

	bind_vao(data.vao);
	vao_enable_attribute(data.vao, data.vbo, 0, 3, GL_FLOAT, sizeof(vertex_t), offsetof(vertex_t, position));
	vao_enable_attribute(data.vao, data.vbo, 1, 2, GL_FLOAT, sizeof(vertex_t), offsetof(vertex_t, tex_coord));
	bind_ebo(data.ebo);
	unbind_vao();
	unbind_ebo();

	data.shader = create_shader("C:/Sarthak/projects/game/resources/shaders/text.vert", "C:/Sarthak/projects/game/resources/shaders/text.frag");
	glm::mat4 projection = glm::ortho(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT);
	shader_set_mat4(data.shader, "projection", projection);
	shader_set_int(data.shader, "char", 0);
}

text_dim_t get_text_dimensions(const char* text) {
	text_dim_t running_dim;
	for (int i = 0; i < strlen(text); i++) {
		unsigned char c = text[i];	
		font_char_t& fc = chars[c];
		running_dim.width += fc.advance;
		running_dim.height = fmax(running_dim.height, fc.height);
		running_dim.height_below_baseline = fmax(fc.height - fc.bearing.y, running_dim. height_below_baseline);
	}
	return running_dim;
}

void draw_text(const char* text, glm::vec2 starting_pos) {
	glm::vec3 color = glm::vec3(240,216,195);
	shader_set_vec3(font_char_t::ui_opengl_data.shader, "color", color / glm::vec3(255.f));
	vertex_t updated_vertices[4];
	glm::vec2 origin = starting_pos;
	for (int i = 0; i < strlen(text); i++) {
		unsigned char c = text[i];	
		font_char_t& fc = chars[c];
		// shader_set_vec3(font_char_t::ui_opengl_data.shader, "color", glm::vec3(0,1,1));
		bind_texture(fc.texture_handle, true);

		glm::vec2 running_pos;
		running_pos.x = origin.x + fc.bearing.x + (fc.width / 2);
		running_pos.y = origin.y + fc.bearing.y - (fc.height / 2);

		updated_vertices[0] = create_vertex(glm::vec3(running_pos.x + (fc.width / 2), running_pos.y + (fc.height / 2), 0.0f), glm::vec3(0,1,1), glm::vec2(1,0)); // top right
		updated_vertices[1] = create_vertex(glm::vec3(running_pos.x + (fc.width / 2), running_pos.y - (fc.height / 2), 0.0f), glm::vec3(0,0,1), glm::vec2(1,1)); // bottom right
		updated_vertices[2] = create_vertex(glm::vec3(running_pos.x - (fc.width / 2), running_pos.y - (fc.height / 2), 0.0f), glm::vec3(0,1,0), glm::vec2(0,1)); // bottom left
		updated_vertices[3] = create_vertex(glm::vec3(running_pos.x - (fc.width / 2), running_pos.y + (fc.height / 2), 0.0f), glm::vec3(1,0,0), glm::vec2(0,0)); // top left
		update_vbo_data(font_char_t::ui_opengl_data.vbo, (float*)updated_vertices, sizeof(updated_vertices));

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		// draw the rectangle render after setting all shader parameters
		draw_obj(font_char_t::ui_opengl_data);
		origin.x += fc.advance;
	}
}