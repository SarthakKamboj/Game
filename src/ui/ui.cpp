#include "ui.h"
#include "constants.h"
#include <unordered_map>

#include "renderer/basic/shape_renders.h"
#include "renderer/opengl/vertex.h"

#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H  

#include "input/input.h"

extern input::user_input_t input_state;

static int cur_widget_count = 0;

static std::vector<int> widget_stack;
static std::vector<widget_t> cur_frame_widgets;

static std::vector<style_t> styles_stack;

void start_of_frame() {
    styles_stack.clear();
    style_t default_style;
    styles_stack.push_back(default_style);

    cur_frame_widgets.clear();
    widget_stack.clear();
    cur_widget_count = 0;

}

void push_style(style_t& style) {
    styles_stack.push_back(style);
}

void pop_style() {
    assert(styles_stack.size() > 1);
    styles_stack.pop_back();
}

void push_widget(int widget_handle) {
    widget_stack.push_back(widget_handle);
}

void pop_widget() {
    widget_stack.pop_back();
}

// const float panel_t::WIDTH = WINDOW_WIDTH;
// const float panel_t::HEIGHT = WINDOW_HEIGHT;

static font_mode_t font_modes[2] = {
    font_mode_t {
        TEXT_SIZE::TITLE,
        56 
    },
    font_mode_t {
        TEXT_SIZE::REGULAR,
        40 
    }
};

// std::vector<panel_t> panels;
// static int panel_handle = 0;

#if 0
int create_panel(style_t& style) {
    panel_t panel;
    panel.handle = panel_handle;
    panel.styling = style;
    panel_handle++;
    panels.push_back(panel);
    return panel.handle;
}
#endif

void register_widget(widget_t& widget, bool push_onto_stack) {
    widget.handle = cur_widget_count++;
    if (widget_stack.size() > 0) widget.parent_widget_handle = widget_stack[widget_stack.size() - 1];
    else widget.parent_widget_handle = -1;
    if (widget.parent_widget_handle != -1) {
        cur_frame_widgets[widget.parent_widget_handle].children_widget_handles.push_back(widget.handle);
    }
    widget.style = styles_stack[styles_stack.size() - 1];
    cur_frame_widgets.push_back(widget);
    if (push_onto_stack) {
        widget_stack.push_back(widget.handle);
    }
}

void create_panel(const char* panel_name) {
    widget_t panel;
    memcpy(panel.key, panel_name, strlen(panel_name)); 
    panel.height = WINDOW_HEIGHT;
    panel.width = WINDOW_WIDTH;
    panel.widget_size = WIDGET_SIZE::PIXEL_BASED;
    register_widget(panel, true);
}

void end_panel() {
    pop_widget();
}

void create_text(const char* text, TEXT_SIZE text_size) {
    widget_t widget;
    widget.text_based = true;
    memcpy(widget.text_info.text, text, fmin(sizeof(widget.text_info.text), strlen(text)));
    widget.text_info.text_size = text_size;
    register_widget(widget);
}

// void add_text_to_panel(int panel_handle, text_t& text) {
//     for (panel_t& panel : panels) {
//         if (panel.handle == panel_handle) {
//             panel.texts.push_back(text);
//         }
//     }
// }

// container_t create_container(style_t& style) {
//     container_t container;
//     container.styling = style;
//     return container;
// }

struct helper_info_t {
    int content_width = 0;
    int content_height = 0;
};

helper_info_t autolayout_hierarchy_helper(int widget_handle, int left_x_pos, int top_y_pos) {
    widget_t& widget = cur_frame_widgets[widget_handle];
    if (widget.text_based) {
        text_dim_t text_dim = get_text_dimensions(widget.text_info.text, widget.text_info.text_size);
        widget.render_x = left_x_pos;
        widget.render_y = top_y_pos;
        widget.render_width = text_dim.width;
        widget.render_height = text_dim.height;

        helper_info_t helper_info;
        helper_info.content_width = text_dim.width;
        helper_info.content_height = text_dim.height;
        return helper_info;
    }

    helper_info_t helper_info;
    helper_info.content_width = widget.width;
    helper_info.content_height = widget.height;

    glm::vec2 running_pos(left_x_pos, top_y_pos);
    for (int child_widget_handle : widget.children_widget_handles) {
        helper_info_t child_helper_info = autolayout_hierarchy_helper(child_widget_handle, running_pos.x, running_pos.y);
        running_pos.y -= child_helper_info.content_height;
    }

    return helper_info;
}

void autolayout_hierarchy() {
    assert(widget_stack.size() == 0);
    for (int i = 0; i < cur_frame_widgets.size(); i++) {
        widget_t& cur_widget = cur_frame_widgets[i];
        if (cur_widget.parent_widget_handle != -1) continue;
        autolayout_hierarchy_helper(cur_widget.handle, 0, WINDOW_HEIGHT);
    }
}

void render_ui() {
    
    for (widget_t& widget : cur_frame_widgets) {
        if (widget.text_based) {
            draw_text(widget.text_info.text, glm::vec2(widget.render_x, widget.render_y), widget.text_info.text_size);
        }
    }
    
    // for (panel_t& panel : panels) {
    //     float content_height = 0;
    //     for (text_t& text : panel.texts) {
    //         text_dim_t dim = get_text_dimensions(text.text, text.styling.text_size);
    //         content_height += dim.height + (text.styling.margins.y * 2) + panel.styling.content_spacing;
    //     }
    //     content_height -= panel.styling.content_spacing;

    //     glm::vec2 prev_anchor_point(0, panel_t::HEIGHT);
    //     if (panel.styling.center_y) {
    //         prev_anchor_point.y -= panel_t::HEIGHT / 2;
    //         prev_anchor_point.y += content_height / 2;
    //     }

    //     for (text_t& text : panel.texts) {
    //         text_dim_t text_dim = get_text_dimensions(text.text, text.styling.text_size);
    //         glm::vec2 cur_render_origin = prev_anchor_point - glm::vec2(0, text_dim.height + text.styling.margins.y);
    //         if (panel.styling.center_x) {
    //             cur_render_origin.x = (panel_t::WIDTH / 2) - (text_dim.width / 2);
    //         }
    //         draw_text(text.text, cur_render_origin, text.styling.text_size);
    //         prev_anchor_point.y -= (text_dim.height + panel.styling.content_spacing + (text.styling.margins.y * 2));
    //     }
    // }
}

void clear_ui() {
    // panels.clear();
    // panel_handle = 0;    
}

opengl_object_data font_char_t::ui_opengl_data{};

void init_fonts() {
    // int font_sizes
    for (font_mode_t& font_mode : font_modes) {
        std::unordered_map<unsigned char, font_char_t>& chars = font_mode.chars;
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
        FT_Set_Pixel_Sizes(face, 0, font_mode.font_size);
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

text_dim_t get_text_dimensions(const char* text, TEXT_SIZE text_size) {
	text_dim_t running_dim;
    for (font_mode_t& font_mode : font_modes) {
        if (font_mode.text_size == text_size) {
            for (int i = 0; i < strlen(text); i++) {
                unsigned char c = text[i];	
                font_char_t& fc = font_mode.chars[c];
                running_dim.width += fc.advance;
                running_dim.height = fmax(running_dim.height, fc.height);
                running_dim.height_below_baseline = fmax(fc.height - fc.bearing.y, running_dim. height_below_baseline);
            }
        }
    }
	return running_dim;
}

void draw_text(const char* text, glm::vec2 starting_pos, TEXT_SIZE text_size) {
	glm::vec3 color = glm::vec3(240,216,195);
	shader_set_vec3(font_char_t::ui_opengl_data.shader, "color", color / glm::vec3(255.f));
	vertex_t updated_vertices[4];
    text_dim_t text_dim = get_text_dimensions(text, text_size);
	glm::vec2 origin = starting_pos - glm::vec2(0, text_dim.height);
    for (font_mode_t& font_mode : font_modes) {
        if (font_mode.text_size == text_size) {
            for (int i = 0; i < strlen(text); i++) {
                unsigned char c = text[i];	
                font_char_t& fc = font_mode.chars[c];
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
    }
}