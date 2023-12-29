#include "ui.h"
#include "constants.h"
#include <unordered_map>

#include "renderer/basic/shape_renders.h"
#include "renderer/opengl/vertex.h"

#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H  

#include "input/input.h"
#include "utils/io.h"

extern input::user_input_t input_state;

static int cur_widget_count = 0;

static std::vector<int> widget_stack;
static std::vector<widget_t> widgets_arr;

static std::vector<style_t> styles_stack;

static std::unordered_map<int, constraint_var_t> constraint_vars;
static std::vector<constraint_t> constraints;

static int constraint_running_cnt = 0;

static bool ui_will_update = false;

glm::vec3 create_color(float r, float g, float b) {
    return glm::vec3(r, g, b) / 255.f;
}

void start_of_frame(bool _ui_will_update) {
    ui_will_update = _ui_will_update;

    styles_stack.clear();
    style_t default_style;
    styles_stack.push_back(default_style);

    if (ui_will_update) {
        widgets_arr.clear();
    }
    cur_widget_count = 0;

    constraint_vars.clear();
    constraints.clear();
    constraint_running_cnt = 0;
}

void push_style(style_t& style) {
    styles_stack.push_back(style);
}

void pop_style() {
    // assert(styles_stack.size() > 1);
    if (styles_stack.size() == 0) {
        std::cout << "styles stack is empty" << std::endl;
        return;
    }
    styles_stack.pop_back();
}

void push_widget(int widget_handle) {
    widget_stack.push_back(widget_handle);
}

void pop_widget() {
    if (widget_stack.size() > 0) widget_stack.pop_back();
}

static font_mode_t font_modes[2] = {
    font_mode_t {
        TEXT_SIZE::TITLE,
        45
    },
    font_mode_t {
        TEXT_SIZE::REGULAR,
        30 
    }
};

int register_widget(widget_t& widget, bool push_onto_stack) {
    widget.handle = cur_widget_count++;
    if (!ui_will_update) return widget.handle;
    if (widget_stack.size() > 0) widget.parent_widget_handle = widget_stack[widget_stack.size() - 1];
    else widget.parent_widget_handle = -1;
    if (widget.parent_widget_handle != -1) {
        widgets_arr[widget.parent_widget_handle].children_widget_handles.push_back(widget.handle);
    }
    widget.style = styles_stack[styles_stack.size() - 1];
    widgets_arr.push_back(widget);
    if (push_onto_stack) {
        widget_stack.push_back(widget.handle);
    }
    return widget.handle;
}

void create_panel(const char* panel_name) {
    widget_t panel;
    memcpy(panel.key, panel_name, strlen(panel_name)); 
    panel.height = WINDOW_HEIGHT;
    panel.width = WINDOW_WIDTH;
    panel.widget_size = WIDGET_SIZE::PIXEL_BASED;

    panel.render_x = 0;
    panel.render_y = WINDOW_HEIGHT;
    panel.render_width = panel.width;
    panel.render_height = panel.height;

    register_widget(panel, true);
}

void end_panel() {
    pop_widget();
}

void create_container(float width, float height, WIDGET_SIZE widget_size) {
    widget_t container;
    const char* container_name = "container";
    memcpy(container.key, container_name, strlen(container_name));
    container.height = height;
    container.width = width;
    container.widget_size = widget_size;

    register_widget(container, true); 
}

void end_container() {
    pop_widget();
}

void create_text(const char* text, TEXT_SIZE text_size) {
    widget_t widget;
    widget.text_based = true;
    memcpy(widget.text_info.text, text, fmin(sizeof(widget.text_info.text), strlen(text)));
    widget.text_info.text_size = text_size;

    text_dim_t text_dim = get_text_dimensions(text, text_size);

    widget.widget_size = WIDGET_SIZE::PIXEL_BASED;
    widget.width = text_dim.width;
    widget.height = text_dim.height;

    widget.render_width = text_dim.width;
    widget.render_height = text_dim.height;

    register_widget(widget);
}

bool create_button(const char* text, TEXT_SIZE text_size) {
    widget_t widget;
    widget.text_based = true;
    memcpy(widget.text_info.text, text, fmin(sizeof(widget.text_info.text), strlen(text)));
    widget.text_info.text_size = text_size;

    text_dim_t text_dim = get_text_dimensions(text, text_size);

    widget.widget_size = WIDGET_SIZE::PIXEL_BASED;
    style_t& latest_style = styles_stack[styles_stack.size() - 1];
    widget.width = text_dim.width + (2 * latest_style.padding.x);
    widget.height = text_dim.height + (2 * latest_style.padding.y);

    widget.render_width = widget.width;
    widget.render_height = widget.height;

    widget.properties = UI_PROPERTIES::CLICKABLE;

    int widget_handle = register_widget(widget);

    if (!ui_will_update) {
        widget_t& cached_widget = widgets_arr[widget_handle];
        if (input_state.x_pos >= cached_widget.render_x &&
            input_state.x_pos <= (cached_widget.render_x + cached_widget.render_width) &&
            // render x and render y specified as the top left pivot and y in ui is 0 on the
            // bottom and WINDOW_HEIGHT on the top, so cached_widget.render_y is the top y of the 
            // widget and cached_widget.render_y - cached_widget.render_height is the bottom y 
            // of the widget
            input_state.y_pos <= cached_widget.render_y &&
            input_state.y_pos >= (cached_widget.render_y - cached_widget.render_height)
        ) {
            return input_state.left_clicked;
        }
        return false;
    }

    return false;
}

struct helper_info_t {
    int content_width = 0;
    int content_height = 0;
};

helper_info_t resolve_positions(int widget_handle, int x_pos_handle, int y_pos_handle) {
    widget_t& widget = widgets_arr[widget_handle];
    if (widget.text_based) {
        // text_dim_t text_dim = get_text_dimensions(widget.text_info.text, widget.text_info.text_size);
        // widget.render_width = text_dim.width;
        // widget.render_height = text_dim.height;

        helper_info_t helper_info;
        helper_info.content_width = widget.render_width;
        helper_info.content_height = widget.render_height;
        return helper_info;
    }

    int space_var = create_constraint_var("spacing", NULL);
    int start_offset = create_constraint_var("start offset", NULL);

    int idx = 1;
    glm::vec2 content_size(0);
    for (int child_widget_handle : widget.children_widget_handles) {

        widget_t& child_widget = widgets_arr[child_widget_handle];

        std::vector<constraint_term_t> terms;
        terms.push_back(create_constraint_term(space_var, idx));
        terms.push_back(create_constraint_term(start_offset, 1));

        int child_widget_x_pos_handle = create_constraint_var("widget_x", &child_widget.render_x);
        int child_widget_y_pos_handle = create_constraint_var("widget_y", &child_widget.render_y);

        if (widget.style.display_dir == DISPLAY_DIR::HORIZONTAL) {
            float constant = content_size.x;
            terms.push_back(create_constraint_term(x_pos_handle, 1));
            create_constraint(child_widget_x_pos_handle, terms, constant);

            std::vector<constraint_term_t> y_pos_terms;
            y_pos_terms.push_back(create_constraint_term(y_pos_handle, 1));
            create_constraint(child_widget_y_pos_handle, y_pos_terms, 0);
        } else {
            float constant = -content_size.y;
            terms.push_back(create_constraint_term(y_pos_handle, 1));
            create_constraint(child_widget_y_pos_handle, terms, constant);

            std::vector<constraint_term_t> x_pos_terms;
            x_pos_terms.push_back(create_constraint_term(x_pos_handle, 1));
            create_constraint(child_widget_x_pos_handle, x_pos_terms, 0);
        }

        helper_info_t child_helper_info = resolve_positions(child_widget_handle, child_widget_x_pos_handle, child_widget_y_pos_handle);
        content_size.x += child_helper_info.content_width;
        content_size.y += child_helper_info.content_height;
        idx++;
    }

    resolve_constraints();

    if (widget.style.display_dir == DISPLAY_DIR::HORIZONTAL) {
        switch (widget.style.float_val) {
            case FLOAT::SPACED_OUT: {
                make_constraint_value_constant(space_var, (widget.render_width - content_size.x) / (widget.children_widget_handles.size() + 1.f));
                make_constraint_value_constant(start_offset, 0);
            }
                break;
            case FLOAT::CENTER: {
                make_constraint_value_constant(space_var, widget.style.content_spacing);
                float remaining_space = widget.render_width - content_size.x - (widget.style.content_spacing * (widget.children_widget_handles.size() + 1.f));
                float offset = remaining_space / 2.f;
                make_constraint_value_constant(start_offset, offset);
            }
                break;
            case FLOAT::START: {
                make_constraint_value_constant(space_var, widget.style.content_spacing);
                make_constraint_value_constant(start_offset, 0);
            }
                break;
            case FLOAT::END: {
                make_constraint_value_constant(space_var, widget.style.content_spacing);
                float offset = (widget.render_width - content_size.x) - (widget.style.content_spacing * (widget.children_widget_handles.size() + 1.f));
                make_constraint_value_constant(start_offset, offset);
            }
                break;
            default: {
                make_constraint_value_constant(space_var, widget.style.content_spacing);
                make_constraint_value_constant(start_offset, 0);
            }
        }
    } else {
        // because pivot is top left but screen coordinate's pivot is bottom left, so y must be negated
        switch (widget.style.float_val) {
            case FLOAT::SPACED_OUT: {
                make_constraint_value_constant(space_var, -(widget.render_height - content_size.y) / (widget.children_widget_handles.size() + 1.f));
                make_constraint_value_constant(start_offset, 0);
            }
                break;
            case FLOAT::CENTER: {
                make_constraint_value_constant(space_var, -widget.style.content_spacing);
                float remaining_space = widget.render_height - content_size.y - (widget.style.content_spacing * (widget.children_widget_handles.size() + 1.f));
                float offset = remaining_space / 2.f;
                make_constraint_value_constant(start_offset, -offset);
            }
                break;
            case FLOAT::START: {
                make_constraint_value_constant(space_var, -widget.style.content_spacing);
                make_constraint_value_constant(start_offset, 0);
            }
                break;
            case FLOAT::END: {
                make_constraint_value_constant(space_var, -widget.style.content_spacing);
                float space_on_top = (widget.render_height - content_size.y) - (widget.style.content_spacing * (widget.children_widget_handles.size() + 1.f));
                make_constraint_value_constant(start_offset, -space_on_top);
            }
                break;
            default: {
                make_constraint_value_constant(space_var, -widget.style.content_spacing);
                make_constraint_value_constant(start_offset, 0);
            }
        }
    }

    helper_info_t helper_info;
    helper_info.content_width = widget.render_width;
    helper_info.content_height = widget.render_height;

    return helper_info;
}

helper_info_t resolve_dimensions(int cur_widget_handle, int parent_width_handle, int parent_height_handle) {
    widget_t& widget = widgets_arr[cur_widget_handle];
    if (widget.text_based) {
        text_dim_t text_dim = get_text_dimensions(widget.text_info.text, widget.text_info.text_size);
        widget.width = text_dim.width + (widget.style.padding.x * 2);
        widget.height = text_dim.height + (widget.style.padding.y * 2);

        widget.render_width = widget.width;
        widget.render_height = widget.height;

        helper_info_t helper_info;
        helper_info.content_width = widget.render_width;
        helper_info.content_height = widget.render_height;
        return helper_info;
    }

    int widget_width_handle = create_constraint_var("width", &widget.render_width);
    int widget_height_handle = create_constraint_var("height", &widget.render_height); 

    glm::vec2 content_size(0);
    for (int child_widget_handle : widget.children_widget_handles) {
        widget_t& child_widget = widgets_arr[child_widget_handle]; 

        helper_info_t child_helper_info = resolve_dimensions(child_widget_handle, widget_width_handle, widget_height_handle);
    
        if (widget.style.display_dir == DISPLAY_DIR::HORIZONTAL) {
            content_size.x += child_helper_info.content_width;
            content_size.y = fmax(content_size.y, child_helper_info.content_height);
        } else {
            content_size.y += child_helper_info.content_height;
            content_size.x = fmax(content_size.x, child_helper_info.content_width); 
        }
    }

    if (widget.style.display_dir == DISPLAY_DIR::HORIZONTAL) {
        content_size.x += widget.style.content_spacing * (widget.children_widget_handles.size() + 1);
    } else {
        content_size.y += widget.style.content_spacing * (widget.children_widget_handles.size() + 1);
    }

    if (parent_width_handle == -1 || parent_height_handle == -1) {
        // this is a parent widget and must be specified in pixels
        assert(widget.widget_size == WIDGET_SIZE::PIXEL_BASED);
        assert(widget.width >= 0.f);
        assert(widget.height >= 0.f);

        make_constraint_value_constant(widget_width_handle, widget.width);
        make_constraint_value_constant(widget_height_handle, widget.height);
    }
    else {
        assert(parent_width_handle != -1);
        assert(parent_height_handle != -1);
        if (widget.widget_size == WIDGET_SIZE::PIXEL_BASED) {
            assert(widget.width >= 0.f);
            assert(widget.height >= 0.f);

            make_constraint_value_constant(widget_width_handle, widget.width);
            make_constraint_value_constant(widget_height_handle, widget.height);
        } else if (widget.widget_size == WIDGET_SIZE::PARENT_PERCENT_BASED) {
            assert(widget.width <= 1.f);
            assert(widget.width >= 0.f);
            assert(widget.height <= 1.f);
            assert(widget.height >= 0.f);

            std::vector<constraint_term_t> width_terms;
            width_terms.push_back(create_constraint_term(parent_width_handle, widget.width));
            create_constraint(widget_width_handle, width_terms, 0);

            std::vector<constraint_term_t> height_terms;
            height_terms.push_back(create_constraint_term(parent_height_handle, widget.height));
            create_constraint(widget_height_handle, height_terms, 0);
        } else if (widget.widget_size == WIDGET_SIZE::FIT_CONTENT) {
            make_constraint_value_constant(widget_width_handle, content_size.x);
            make_constraint_value_constant(widget_height_handle, content_size.y);
        }
    }

    resolve_constraints();

    helper_info_t helper_info;
    helper_info.content_width = widget.render_width;
    helper_info.content_height = widget.render_height;

    return helper_info;    
}

void autolayout_hierarchy() {

    if (!ui_will_update) return;

    assert(widget_stack.size() == 0);

    for (int i = 0; i < widgets_arr.size(); i++) {
        widget_t& cur_widget = widgets_arr[i];
        if (cur_widget.parent_widget_handle != -1) continue;
        int width_var = create_constraint_var_constant(cur_widget.render_width);
        int height_var = create_constraint_var_constant(cur_widget.render_height);
        resolve_dimensions(cur_widget.handle, -1, -1);
    }

    for (int i = 0; i < widgets_arr.size(); i++) {
        widget_t& cur_widget = widgets_arr[i];
        if (cur_widget.parent_widget_handle != -1) continue;
        int x_var = create_constraint_var_constant(cur_widget.render_x);
        int y_var = create_constraint_var_constant(cur_widget.render_y);
        resolve_positions(cur_widget.handle, x_var, y_var);
    }

    resolve_constraints();
    ui_will_update = false;
}

int create_constraint_var(const char* var_name, float* val) {
    constraint_var_t constraint_value;
    constraint_value.handle = constraint_running_cnt++;
    memcpy(constraint_value.name, var_name, strlen(constraint_value.name));
    constraint_value.constant = false;
    constraint_value.value = val;
    constraint_value.cur_val = 0;
    constraint_vars[constraint_value.handle] = constraint_value;
    return constraint_value.handle;
}

int create_constraint_var_constant(float value) {
    constraint_var_t constraint_value;
    constraint_value.handle = constraint_running_cnt++;
    constraint_value.constant = true;
    constraint_value.cur_val = value;
    constraint_vars[constraint_value.handle] = constraint_value;
    return constraint_value.handle;
}

constraint_term_t create_constraint_term(int var_handle, float coefficient) {
    constraint_term_t term;
    term.coefficient = coefficient;
    term.var_handle = var_handle;
    return term;
}

void make_constraint_value_constant(int constraint_handle, float value) {
    constraint_var_t& constraint_var = constraint_vars[constraint_handle];
    constraint_var.constant = true;
    constraint_var.cur_val = value;
    if (constraint_var.value) *constraint_var.value = value;
}

void create_constraint(int constraint_var_handle, std::vector<constraint_term_t>& right_side_terms, float constant) {
    constraint_t constraint;
    constraint.left_side_var_handle = constraint_var_handle;
    for (constraint_term_t& right_side : right_side_terms) {
        constraint.right_side.push_back(right_side);
    }
    constraint.constant_handles.push_back(create_constraint_var_constant(constant));
    constraints.push_back(constraint);
}

void resolve_constraints() {
    bool something_changed = false;
    do {
        something_changed = false;
        for (constraint_t& constraint : constraints) {
            constraint_var_t& left_value = constraint_vars[constraint.left_side_var_handle];
            if (left_value.constant) continue;
            float running_right_side = 0.f;
            bool constaint_resolved = true;
            for (constraint_term_t& term : constraint.right_side) {
                if (constraint_vars[term.var_handle].constant) {
                    running_right_side += constraint_vars[term.var_handle].cur_val * term.coefficient;
                } else {
                    constaint_resolved = false;
                    break;
                }
            } 
            if (constaint_resolved) {
                for (int const_handle : constraint.constant_handles) {
                    running_right_side += constraint_vars[const_handle].cur_val;
                }
                make_constraint_value_constant(constraint.left_side_var_handle, running_right_side);
                something_changed = true;
            }
        }
    } while (something_changed);
}

void render_ui_helper(widget_t& widget) {

    if (widget.style.background_color != TRANSPARENT_COLOR) {
        draw_background(widget);
    }
    if (widget.text_based) { 
        draw_text(widget.text_info.text, glm::vec2(widget.render_x + widget.style.padding.x, widget.render_y - widget.style.padding.y), widget.text_info.text_size);
    } 

    for (int child_handle : widget.children_widget_handles) {
        render_ui_helper(widgets_arr[child_handle]);
    }
}

void render_ui() { 
    for (widget_t& widget : widgets_arr) {
        if (widget.parent_widget_handle == -1) {
            render_ui_helper(widget);
        }
    }
}

opengl_object_data font_char_t::ui_opengl_data{};

void init_ui() {
    // int font_sizes
    for (font_mode_t& font_mode : font_modes) {
        std::unordered_map<unsigned char, font_char_t>& chars = font_mode.chars;
        FT_Library lib;
        if (FT_Init_FreeType(&lib)) {
            std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return;
        }
        FT_Face face;

        char resource_path[256]{};
        io::get_resources_folder_path(resource_path);
        char font_path[256]{};
        sprintf(font_path, "%s\\%s\\Courier_Prime\\CourierPrime-Regular.ttf", resource_path, FONTS_FOLDER);

        if (FT_New_Face(lib, font_path, 0, &face))
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

	data.shader = create_shader("text.vert", "text.frag");
	glm::mat4 projection = glm::ortho(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT);
	shader_set_mat4(data.shader, "projection", projection);
	shader_set_int(data.shader, "char", 0);

    if (detect_gl_error()) {
		std::cout << "error loading the ui data" << std::endl;
	} else {
		std::cout << "successfully init ui data" << std::endl;
    }
}

text_dim_t get_text_dimensions(const char* text, TEXT_SIZE text_size) {
	text_dim_t running_dim;
    for (font_mode_t& font_mode : font_modes) {
        if (font_mode.text_size == text_size) {
            for (int i = 0; i < strlen(text); i++) {
                unsigned char c = text[i];	
                font_char_t& fc = font_mode.chars[c];
                running_dim.width += fc.advance;
                running_dim.max_height_above_baseline = fmax(fc.bearing.y, running_dim.max_height_above_baseline);
                running_dim.max_height_below_baseline = fmax(fc.height - fc.bearing.y, running_dim.max_height_below_baseline);
            }
        }
    }
    running_dim.height = running_dim.max_height_above_baseline + running_dim.max_height_below_baseline;
	return running_dim;
}

void draw_background(widget_t& widget) {
	shader_set_vec3(font_char_t::ui_opengl_data.shader, "color", widget.style.background_color);
	shader_set_float(font_char_t::ui_opengl_data.shader, "tex_influence", 0.f);
	shader_set_int(font_char_t::ui_opengl_data.shader, "round_vertices", 1);
	shader_set_float(font_char_t::ui_opengl_data.shader, "border_radius", widget.style.border_radius);

    float x = widget.render_x;
    float y = widget.render_y;
    float width = widget.render_width;
    float height = widget.render_height;

	vertex_t updated_vertices[4];
#if 0
	glm::vec2 origin = glm::vec2(x + (width / 2), y - (height / 2));
    updated_vertices[0] = create_vertex(glm::vec3(x + (width / 2), y + (height / 2), 0.0f), glm::vec3(0,1,1), glm::vec2(0,0)); // top right
    updated_vertices[1] = create_vertex(glm::vec3(x + (width / 2), y - (height / 2), 0.0f), glm::vec3(0,0,1), glm::vec2(0,0)); // bottom right
    updated_vertices[2] = create_vertex(glm::vec3(x - (width / 2), y - (height / 2), 0.0f), glm::vec3(0,1,0), glm::vec2(0,0)); // bottom left
    updated_vertices[3] = create_vertex(glm::vec3(x - (width / 2), y + (height / 2), 0.0f), glm::vec3(1,0,0), glm::vec2(0,0)); // top left
#else
	glm::vec3 origin = glm::vec3(x, y, 0);
    updated_vertices[0] = create_vertex(origin + glm::vec3(width, 0, 0.0f), glm::vec3(0,1,1), glm::vec2(0,0)); // top right
    updated_vertices[1] = create_vertex(origin + glm::vec3(width, -height, 0.0f), glm::vec3(0,0,1), glm::vec2(0,0)); // bottom right
    updated_vertices[2] = create_vertex(origin + glm::vec3(0, -height, 0.0f), glm::vec3(0,1,0), glm::vec2(0,0)); // bottom left
    updated_vertices[3] = create_vertex(origin, glm::vec3(1,0,0), glm::vec2(0,0)); // top left
#endif
    update_vbo_data(font_char_t::ui_opengl_data.vbo, (float*)updated_vertices, sizeof(updated_vertices));

    shader_set_vec3(font_char_t::ui_opengl_data.shader, "top_left", updated_vertices[3].position);
    shader_set_vec3(font_char_t::ui_opengl_data.shader, "top_right", updated_vertices[0].position);
    shader_set_vec3(font_char_t::ui_opengl_data.shader, "bottom_left", updated_vertices[2].position);
    shader_set_vec3(font_char_t::ui_opengl_data.shader, "bottom_right", updated_vertices[1].position);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // draw the rectangle render after setting all shader parameters
    draw_obj(font_char_t::ui_opengl_data);
}

void draw_text(const char* text, glm::vec2 starting_pos, TEXT_SIZE text_size) {
	glm::vec3 color = create_color(240,216,195);
	shader_set_vec3(font_char_t::ui_opengl_data.shader, "color", color);
	shader_set_float(font_char_t::ui_opengl_data.shader, "tex_influence", 1.f);
	shader_set_int(font_char_t::ui_opengl_data.shader, "round_vertices", 0);
    text_dim_t text_dim = get_text_dimensions(text, text_size);
	glm::vec2 origin = starting_pos - glm::vec2(0, text_dim.max_height_above_baseline);
	vertex_t updated_vertices[4];
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