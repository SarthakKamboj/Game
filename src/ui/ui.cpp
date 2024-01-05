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

#include "app.h"

extern input::user_input_t input_state;
extern application_t app;

static int cur_widget_count = 0;

static std::vector<int> widget_stack;
static std::vector<widget_t> widgets_arr;

static std::vector<style_t> styles_stack;

static std::unordered_map<int, constraint_var_t> constraint_vars;
static std::vector<constraint_t> constraints;

static int constraint_running_cnt = 0;

static bool ui_will_update = false;

uint32_t wrap(uint32_t in, int num) {
    uint32_t res = 0;
    res = in >> num;
    uint32_t top = in << (32 - num);
    res = res | num;
    return res;
}

uint32_t bitwise_add_mod2(uint32_t a, uint32_t b, uint32_t c) {
    // return (~a & ~b & c) | (~a & b & ~c) | (a & b & c) | (a & ~b & ~c);
    return a ^ b ^ c;
}

uint32_t epsilon0(uint32_t in) {
    uint32_t wrap7 = wrap(in, 7);
    uint32_t wrap18 = wrap(in, 18);
    uint32_t shift3 = in >> 3; 
    return bitwise_add_mod2(wrap7, wrap18, shift3);
}

uint32_t epsilon1(uint32_t in) {
    uint32_t wrap17 = wrap(in, 17);
    uint32_t wrap19 = wrap(in, 19);
    uint32_t shift10 = in >> 10; 
    return bitwise_add_mod2(wrap17, wrap19, shift10);
}

uint32_t sigma0(uint32_t in) {
    uint32_t wrap2 = wrap(in, 2);
    uint32_t wrap13 = wrap(in, 13);
    uint32_t wrap22 = wrap(in, 22);
    return bitwise_add_mod2(wrap2, wrap13, wrap22);
}

uint32_t sigma1(uint32_t in) {
    uint32_t wrap6 = wrap(in, 6);
    uint32_t wrap11 = wrap(in, 11);
    uint32_t wrap25 = wrap(in, 25);
    return bitwise_add_mod2(wrap6, wrap11, wrap25);
}

uint32_t maj(uint32_t a, uint32_t b, uint32_t c) {
    return (a & b) ^ (a & c) ^ (b & c);
}

uint32_t ch(uint32_t a, uint32_t b, uint32_t c) {
    return (a & b) ^ (~a & c);
}

uint32_t calc_t1(working_variables_t& cur_vars, uint32_t* w, int t) {

    static uint32_t k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    uint64_t interm = cur_vars.h + sigma1(cur_vars.e) + ch(cur_vars.e, cur_vars.f, cur_vars.g) + k[t] + w[t];
    uint32_t t1 = mod_2_pow_32(interm);
    return t1;
}

uint32_t calc_t2(working_variables_t& cur_vars) {
    uint64_t s0 = sigma0(cur_vars.a);
    uint64_t interm = s0 + maj(cur_vars.a, cur_vars.b, cur_vars.c);
    return mod_2_pow_32(interm);
}

uint32_t mod_2_pow_32(uint64_t in) {
    uint64_t div = 1;
    div = div << 32;
    uint64_t interm = in % div;
    uint64_t mask = 0x0000ffff;
    uint32_t res = interm & mask;
    return res;
}

sha256_t sha256_hash(const char* key) {
    uint512_t input{};
    int key_len = strlen(key);
    uint64_t size = sizeof(unsigned char) * key_len;
    assert(size < 512);
    
    memcpy(&input, key, size);

    /*  PADDING */
    // last 8 bytes reserved for the padding size
    input.unsigned_bytes[key_len] = 0x80;
    input.unsigned_double[7] = size;

    uint32_t h[8] = {
        0x6a09e667,
        0xbb67ae85,
        0x3c6ef372,
        0xa54ff53a,
        0x510e527f,
        0x9b05688c,
        0x1f83d9ab,
        0x5be0cd19
    };

    uint32_t w[64]{};
    for (int t = 0; t < 16; t++) {
        w[t] = input.unsigned_ints[t];
    }

    for (int t = 16; t < 63; t++) {
        int64_t ep1 = epsilon1(w[t-2]);
        int64_t ep0 = epsilon0(w[t-15]);
        uint64_t intermediate = ep1 + w[t-7] + ep0 + w[t-16];
        w[t] = mod_2_pow_32(intermediate);
    }

    working_variables_t working_variables{};
    memcpy(&working_variables, h, sizeof(working_variables_t));

    for (int t = 0; t < 63; t++) {
        working_variables_t cur_vars = working_variables;

        uint64_t t1 = calc_t1(cur_vars, w, t);
        uint64_t t2 = calc_t2(cur_vars);

        uint64_t a_interm = t1 + t2;
        working_variables.a = mod_2_pow_32(a_interm);

        working_variables.b = cur_vars.a;
        working_variables.c = cur_vars.b;
        working_variables.d = cur_vars.c;

        uint64_t e_interm = cur_vars.d + t1;
        working_variables.e = mod_2_pow_32(e_interm);

        working_variables.f = cur_vars.e;
        working_variables.g = cur_vars.f;
        working_variables.h = cur_vars.g;

        cur_vars = working_variables;
    }

    for (int i = 0; i < 8; i++) {
        uint64_t v1 = working_variables.vals[i];
        uint64_t v2 = h[i];
        uint64_t v = v1 + v2;
        h[i] = mod_2_pow_32(v);
    }

    sha256_t sha;
    for (int i = 0; i < 8; i++) {
        sha.unsigned_ints[i] = h[i];
    }

    return sha;
}

glm::vec3 create_color(float r, float g, float b) {
    return glm::vec3(r, g, b) / 255.f;
}

void start_of_frame(bool _ui_will_update) {
    ui_will_update = _ui_will_update;

    glm::mat4 projection = glm::ortho(0.0f, app.window_width, 0.0f, app.window_height);
    shader_set_mat4(font_char_t::ui_opengl_data.shader, "projection", projection);
   if (app.resized) {
        ui_will_update = true;
	}

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
    // game_assert(styles_stack.size() > 1);
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
    if (widget_stack.size() > 0) {
        std::cout << "popping off " << widgets_arr[widget_stack[widget_stack.size() - 1]].key << std::endl;
        widget_stack.pop_back();
    }
}

static font_mode_t font_modes[2] = {
    font_mode_t {
        TEXT_SIZE::TITLE,
        45
    },
    font_mode_t {
        TEXT_SIZE::REGULAR,
        20
    }
};

int register_widget(widget_t& widget, const char* key, bool push_onto_stack) {
    widget.handle = cur_widget_count++;
    memcpy(widget.key, key, strlen(key));
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
    panel.height = app.window_height;
    panel.width = app.window_width;
    // panel.widget_size = WIDGET_SIZE::PIXEL_BASED;
    panel.widget_size_width = WIDGET_SIZE::PIXEL_BASED;
    panel.widget_size_height = WIDGET_SIZE::PIXEL_BASED;

    panel.x = 0;
    panel.y = app.window_height;
    panel.render_width = panel.width;
    panel.render_height = panel.height;

    panel.content_width = panel.width;
    panel.content_height = panel.height;

    register_widget(panel, panel_name, true);
}

void end_panel() {
    pop_widget();
}

// void create_container(float width, float height, WIDGET_SIZE widget_size) {
void create_container(float width, float height, WIDGET_SIZE widget_size_width, WIDGET_SIZE widget_size_height, const char* container_name) {
    widget_t container;
    memcpy(container.key, container_name, strlen(container_name));
    container.height = height;
    container.width = width;
    container.widget_size_width = widget_size_width;
    container.widget_size_height = widget_size_height;

    register_widget(container, container_name, true); 
}

void end_container() {
    pop_widget();
}

void create_image_container(int texture_handle, float width, float height, WIDGET_SIZE widget_size_width, WIDGET_SIZE widget_size_height, const char* img_name) {

    game_assert(widget_size_width != WIDGET_SIZE::FIT_CONTENT);
    game_assert(widget_size_height != WIDGET_SIZE::FIT_CONTENT);

    texture_t* tex = get_tex(texture_handle);
    game_assert(tex);
    game_assert(tex->tex_slot == 1);

    widget_t widget;
    widget.image_based = true;
    widget.texture_handle = texture_handle;
    widget.widget_size_width = widget_size_width;
    widget.widget_size_height = widget_size_height;
    widget.width = width;
    widget.height = height;

    register_widget(widget, img_name);
}

bool create_selector(int selected_option, const char** options, int num_options, float width, float height, int& updated_selected_option) {
    style_t container_style;
    container_style.display_dir = DISPLAY_DIR::HORIZONTAL;
    container_style.horizontal_align_val = ALIGN::SPACE_BETWEEN;
    container_style.vertical_align_val = ALIGN::CENTER;
    push_style(container_style);
    create_container(width, height, WIDGET_SIZE::PIXEL_BASED, WIDGET_SIZE::PIXEL_BASED, "selector container");
    pop_style();
    
    bool changed = false;
    
    style_t enabled;
    enabled.color = DARK_BLUE;
    enabled.padding = glm::vec2(10);
    
    style_t disabled;
    disabled.color = GREY;
    disabled.padding = glm::vec2(10);
    
    bool can_go_left = selected_option >= 1;
    if (can_go_left) {
        push_style(enabled);
    } else {
        push_style(disabled);
    }
    
    if (can_go_left) {
        if (create_button("<")) {
            updated_selected_option = selected_option - 1;
            changed = true;
        }
    } else {
        create_text("<");
    }
    pop_style();

    create_text(options[selected_option]);

    bool can_go_right = selected_option <= num_options - 2;
    if (can_go_right) {
        push_style(enabled);
    } else {
        push_style(disabled);
    }
    if (can_go_right) {
        if (create_button(">")) {
            updated_selected_option = selected_option + 1;
            changed = true;
        }
    } else {
        create_text(">");
    }
    pop_style();
    
    end_container();
    return changed;
}

void create_text(const char* text, TEXT_SIZE text_size) {
    widget_t widget;
    widget.text_based = true;
    memcpy(widget.text_info.text, text, fmin(sizeof(widget.text_info.text), strlen(text)));
    widget.text_info.text_size = text_size;

    text_dim_t text_dim = get_text_dimensions(text, text_size);

    widget.widget_size_width = WIDGET_SIZE::PIXEL_BASED;
    widget.widget_size_height = WIDGET_SIZE::PIXEL_BASED;
    
    style_t& latest_style = styles_stack[styles_stack.size() - 1];
    widget.width = text_dim.width + (2 * latest_style.padding.x);
    // widget.height = text_dim.height + (2 * latest_style.padding.y);
    widget.height = text_dim.max_height_above_baseline + (2 * latest_style.padding.y);

    widget.render_width = widget.width;
    widget.render_height = widget.height;

    register_widget(widget, text);
}

bool create_button(const char* text, TEXT_SIZE text_size) {
    widget_t widget;
    widget.text_based = true;
    memcpy(widget.text_info.text, text, fmin(sizeof(widget.text_info.text), strlen(text)));
    widget.text_info.text_size = text_size;

    text_dim_t text_dim = get_text_dimensions(text, text_size);

    widget.widget_size_width = WIDGET_SIZE::PIXEL_BASED;
    widget.widget_size_height = WIDGET_SIZE::PIXEL_BASED;
    style_t& latest_style = styles_stack[styles_stack.size() - 1];
    widget.width = text_dim.width + (2 * latest_style.padding.x);
    // widget.height = text_dim.height + (2 * latest_style.padding.y);
    widget.height = text_dim.max_height_above_baseline + (2 * latest_style.padding.y);

    widget.render_width = widget.width;
    widget.render_height = widget.height;

    widget.properties = UI_PROPERTIES::CLICKABLE;

    int widget_handle = register_widget(widget, text);

    if (!ui_will_update) {
        widget_t& cached_widget = widgets_arr[widget_handle];
        if (input_state.x_pos >= (cached_widget.x + cached_widget.style.margin.x) &&
            input_state.x_pos <= (cached_widget.x + cached_widget.render_width + cached_widget.style.margin.x) &&
            // render x and render y specified as the top left pivot and y in ui is 0 on the
            // bottom and WINDOW_HEIGHT on the top, so cached_widget.y is the top y of the 
            // widget and cached_widget.y - cached_widget.render_height is the bottom y 
            // of the widget
            input_state.y_pos <= (cached_widget.y - cached_widget.style.margin.y) &&
            input_state.y_pos >= (cached_widget.y - cached_widget.render_height - cached_widget.style.margin.y)
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
        // render_width and render_height for text_based things are calculated upon instantiation
        helper_info_t helper_info;
        helper_info.content_width = widget.content_width;
        helper_info.content_height = widget.content_height;
        return helper_info;
    }


    int widget_width_handle = create_constraint_var_constant(widget.content_width);
    int widget_height_handle = create_constraint_var_constant(widget.content_height);

    int space_var_hor = create_constraint_var("spacing hor", NULL);
    int space_var_vert = create_constraint_var("spacing vert", NULL);
    int start_offset_hor = create_constraint_var("start offset hor", NULL);
    int start_offset_vert = create_constraint_var("start offset vert", NULL);

    int idx = 0;
    if (widget.style.display_dir == DISPLAY_DIR::HORIZONTAL && widget.style.horizontal_align_val == ALIGN::SPACE_AROUND ||
        widget.style.display_dir == DISPLAY_DIR::VERTICAL && widget.style.vertical_align_val == ALIGN::SPACE_AROUND) {
        idx++;
    }
    glm::vec2 content_size(0);
    for (int child_widget_handle : widget.children_widget_handles) {

        widget_t& child_widget = widgets_arr[child_widget_handle];
        
        int child_widget_x_pos_handle = create_constraint_var("widget_x", &child_widget.x);
        int child_widget_y_pos_handle = create_constraint_var("widget_y", &child_widget.y);

        if (widget.style.display_dir == DISPLAY_DIR::HORIZONTAL) {
            std::vector<constraint_term_t> x_pos_terms;
            float constant = content_size.x;
            x_pos_terms.push_back(create_constraint_term(x_pos_handle, 1));
            x_pos_terms.push_back(create_constraint_term(start_offset_hor, 1));
            x_pos_terms.push_back(create_constraint_term(space_var_hor, idx));
            create_constraint(child_widget_x_pos_handle, x_pos_terms, constant);

            float vert_constant = 0;
            std::vector<constraint_term_t> y_pos_terms;
            y_pos_terms.push_back(create_constraint_term(y_pos_handle, 1));
            if (widget.style.vertical_align_val == ALIGN::END) {
                y_pos_terms.push_back(create_constraint_term(widget_height_handle, -1.f));
                vert_constant = child_widget.content_height;
            } else {
                y_pos_terms.push_back(create_constraint_term(start_offset_vert, 1));
                if (widget.style.vertical_align_val == ALIGN::CENTER) {
                    y_pos_terms.push_back(create_constraint_term(widget_height_handle, -0.5f));
                    vert_constant = 0.5f * child_widget.content_height;
                }
            }
            create_constraint(child_widget_y_pos_handle, y_pos_terms, vert_constant);
        } else {
            std::vector<constraint_term_t> y_pos_terms;
            float constant = -content_size.y;
            y_pos_terms.push_back(create_constraint_term(y_pos_handle, 1));
            y_pos_terms.push_back(create_constraint_term(start_offset_vert, 1));
            y_pos_terms.push_back(create_constraint_term(space_var_vert, idx));
            create_constraint(child_widget_y_pos_handle, y_pos_terms, constant);

            float hor_constant = 0;
            std::vector<constraint_term_t> x_pos_terms;
            x_pos_terms.push_back(create_constraint_term(x_pos_handle, 1));
            if (widget.style.horizontal_align_val == ALIGN::END) {
                x_pos_terms.push_back(create_constraint_term(widget_width_handle, 1));
                hor_constant = -child_widget.content_width;
            } else {
                x_pos_terms.push_back(create_constraint_term(start_offset_hor, 1));
                if (widget.style.horizontal_align_val == ALIGN::CENTER) {
                    x_pos_terms.push_back(create_constraint_term(widget_width_handle, 0.5f));
                    hor_constant = -0.5f * child_widget.content_width;
                }
            }
            create_constraint(child_widget_x_pos_handle, x_pos_terms, hor_constant);
        }

        helper_info_t child_helper_info = resolve_positions(child_widget_handle, child_widget_x_pos_handle, child_widget_y_pos_handle);
        if (widget.style.display_dir == DISPLAY_DIR::HORIZONTAL) {
            content_size.x += child_helper_info.content_width;
            content_size.y = fmax(content_size.y, child_helper_info.content_height);
        } else {
            content_size.x = fmax(content_size.x, child_helper_info.content_width);
            content_size.y += child_helper_info.content_height;
        }
        idx++;
    }

    resolve_constraints();

    int num_children = widget.children_widget_handles.size();
    switch (widget.style.horizontal_align_val) {
        case ALIGN::SPACE_BETWEEN: {
            if (widget.style.display_dir == DISPLAY_DIR::HORIZONTAL) {
                if (num_children == 1) {
                    make_constraint_value_constant(space_var_hor, (widget.content_width - content_size.x) / 2);
                } else {
                    make_constraint_value_constant(space_var_hor, (widget.content_width - content_size.x) / (num_children - 1));
                }
                make_constraint_value_constant(start_offset_hor, 0);
            } else {
                make_constraint_value_constant(space_var_hor, 0);
                make_constraint_value_constant(start_offset_hor, 0);
            }
        }
            break;
        case ALIGN::SPACE_AROUND: {
            if (widget.style.display_dir == DISPLAY_DIR::HORIZONTAL) {
                make_constraint_value_constant(space_var_hor, (widget.content_width - content_size.x) / (num_children + 1));
                make_constraint_value_constant(start_offset_hor, 0);
            } else {
                make_constraint_value_constant(space_var_hor, 0);
                make_constraint_value_constant(start_offset_hor, 0);
            }
        }
            break;
        case ALIGN::CENTER: {
            make_constraint_value_constant(space_var_hor, widget.style.content_spacing);
            if (widget.style.display_dir == DISPLAY_DIR::HORIZONTAL) {
                float remaining_space = 0.f;
                remaining_space = widget.content_width - content_size.x - (widget.style.content_spacing * (num_children - 1));
                float offset = remaining_space / 2.f;
                make_constraint_value_constant(start_offset_hor, offset);
            } else {
                make_constraint_value_constant(start_offset_hor, 0);
            }
        }
            break;
        case ALIGN::START: {
            make_constraint_value_constant(space_var_hor, widget.style.content_spacing);
            make_constraint_value_constant(start_offset_hor, 0);
        }
            break;
        case ALIGN::END: {
            make_constraint_value_constant(space_var_hor, widget.style.content_spacing);
            float offset = 0.f;
            if (widget.style.display_dir == DISPLAY_DIR::HORIZONTAL) {
                offset = (widget.content_width - content_size.x) - (widget.style.content_spacing * (widget.children_widget_handles.size() + 1.f));
            } else {
                offset = widget.content_width - content_size.x;
            }
            make_constraint_value_constant(start_offset_hor, offset);
        }
            break;
        default: {
            make_constraint_value_constant(space_var_hor, 0);
            make_constraint_value_constant(start_offset_hor, 0);
        }
    }
    
    // because pivot is top left but screen coordinate's pivot is bottom left, so y must be negated
    switch (widget.style.vertical_align_val) {
        case ALIGN::SPACE_BETWEEN: {
            if (widget.style.display_dir == DISPLAY_DIR::VERTICAL) {
                if (num_children == 1) {
                    make_constraint_value_constant(space_var_vert, -(widget.content_height - content_size.y) / 2);
                } else {
                    make_constraint_value_constant(space_var_vert, -(widget.content_height - content_size.y) / (num_children - 1));
                }
                make_constraint_value_constant(start_offset_vert, 0);
            } else {
                make_constraint_value_constant(space_var_vert, 0);
                make_constraint_value_constant(start_offset_vert, 0);
            }
        }
            break;
        case ALIGN::SPACE_AROUND: {
            if (widget.style.display_dir == DISPLAY_DIR::VERTICAL) {
                make_constraint_value_constant(space_var_vert, -(widget.content_height - content_size.y) / (num_children + 1));
                make_constraint_value_constant(start_offset_vert, 0);
            } else {
                make_constraint_value_constant(space_var_vert, 0);
                make_constraint_value_constant(start_offset_vert, 0);
            }
        }
            break;
        case ALIGN::CENTER: {
            make_constraint_value_constant(space_var_vert, -widget.style.content_spacing);
            if (widget.style.display_dir == DISPLAY_DIR::VERTICAL) {
                float remaining_space = remaining_space = widget.content_height - content_size.y - (widget.style.content_spacing * (num_children - 1));
                float offset = remaining_space / 2.f;
                make_constraint_value_constant(start_offset_vert, -offset);
            } else {
                make_constraint_value_constant(start_offset_vert, 0);
            }
        }
            break;
        case ALIGN::START: {
            make_constraint_value_constant(space_var_vert, -widget.style.content_spacing);
            make_constraint_value_constant(start_offset_vert, 0);
        }
            break;
        case ALIGN::END: {
            make_constraint_value_constant(space_var_vert, -widget.style.content_spacing);
            float space_on_top = 0.f;
            if (widget.style.display_dir == DISPLAY_DIR::VERTICAL) {
                space_on_top = (widget.content_height - content_size.y) - (widget.style.content_spacing * (widget.children_widget_handles.size() + 1.f));
            } else {
                space_on_top = widget.content_height - content_size.y;
            }
            make_constraint_value_constant(start_offset_vert, -space_on_top);
        }
            break;
        default: {
            make_constraint_value_constant(space_var_vert, 0);
            make_constraint_value_constant(start_offset_vert, 0);
        }
    }

    resolve_constraints();

    helper_info_t helper_info;
    helper_info.content_width = widget.content_width;
    helper_info.content_height = widget.content_height;

    return helper_info;
}

helper_info_t resolve_dimensions(int cur_widget_handle, int parent_width_handle, int parent_height_handle) {
    widget_t& widget = widgets_arr[cur_widget_handle];
    if (widget.text_based) {
        text_dim_t text_dim = get_text_dimensions(widget.text_info.text, widget.text_info.text_size);
        widget.width = text_dim.width;
        // widget.height = text_dim.height;
        widget.height = text_dim.max_height_above_baseline;

        widget.render_width = widget.width + (widget.style.padding.x * 2);
        widget.render_height = widget.height + (widget.style.padding.y * 2);

        widget.content_width = widget.render_width + (widget.style.margin.x * 2);
        widget.content_height = widget.render_height + (widget.style.margin.y * 2);

        helper_info_t helper_info;
        helper_info.content_width = widget.content_width;
        helper_info.content_height = widget.content_height;
        return helper_info;
    }

    int widget_width_handle = create_constraint_var("width", &widget.render_width);
    int widget_height_handle = create_constraint_var("height", &widget.render_height); 

    if (widget.widget_size_width == WIDGET_SIZE::PIXEL_BASED) {
        game_assert(widget.width >= 0.f);
        float render_width = 0.f;
        if (parent_width_handle == -1 || parent_height_handle == -1) {
            render_width = widget.width;
        } else {
            render_width = widget.width + (widget.style.padding.x * 2);
        }

        make_constraint_value_constant(widget_width_handle, render_width);
    } else if (widget.widget_size_width == WIDGET_SIZE::PARENT_PERCENT_BASED) {
        game_assert(widget.width <= 1.f);
        game_assert(widget.width >= 0.f);

        std::vector<constraint_term_t> width_terms;
        width_terms.push_back(create_constraint_term(parent_width_handle, widget.width));
        create_constraint(widget_width_handle, width_terms, widget.style.padding.x * 2);
    }
    
    if (widget.widget_size_height == WIDGET_SIZE::PIXEL_BASED) {
        game_assert(widget.height >= 0.f);
        float render_height = 0.f;
        if (parent_width_handle == -1 || parent_height_handle == -1) {
            render_height = widget.height;
        } else {
            render_height = widget.height + (widget.style.padding.y * 2);
        }
        make_constraint_value_constant(widget_height_handle, render_height);
    } else if (widget.widget_size_height == WIDGET_SIZE::PARENT_PERCENT_BASED) {
        game_assert(widget.height <= 1.f);
        game_assert(widget.height >= 0.f);

        std::vector<constraint_term_t> height_terms;
        height_terms.push_back(create_constraint_term(parent_height_handle, widget.height));
        create_constraint(widget_height_handle, height_terms, widget.style.padding.y * 2);
    }

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
        helper_info_t helper_info;
        helper_info.content_width = widget.content_width;
        helper_info.content_height = widget.content_height;
        return helper_info;
    }

    game_assert(parent_width_handle != -1);
    game_assert(parent_height_handle != -1);
        
    if (widget.widget_size_width == WIDGET_SIZE::FIT_CONTENT) {
        float render_width = content_size.x + (widget.style.padding.x * 2.f);
        make_constraint_value_constant(widget_width_handle, render_width);
    }
    
    if (widget.widget_size_height == WIDGET_SIZE::FIT_CONTENT) {
        float render_height = content_size.y + (widget.style.padding.y * 2);
        make_constraint_value_constant(widget_height_handle, render_height);
    }

    resolve_constraints();

    widget.content_width = widget.render_width + (widget.style.margin.x * 2);
    widget.content_height = widget.render_height + (widget.style.margin.y * 2);

    helper_info_t helper_info;
    helper_info.content_width = widget.content_width;
    helper_info.content_height = widget.content_height;

    return helper_info;    
}

void autolayout_hierarchy() {

    if (!ui_will_update) return;

    game_assert(widget_stack.size() == 0);
    game_assert(styles_stack.size() == 1);

    for (int i = 0; i < widgets_arr.size(); i++) {
        widget_t& cur_widget = widgets_arr[i];
        if (cur_widget.parent_widget_handle != -1) continue;
        resolve_dimensions(cur_widget.handle, -1, -1);
    }

    for (int i = 0; i < widgets_arr.size(); i++) {
        widget_t& cur_widget = widgets_arr[i];
        if (cur_widget.parent_widget_handle != -1) continue;
        int x_var = create_constraint_var_constant(cur_widget.x);
        int y_var = create_constraint_var_constant(cur_widget.y);
        resolve_positions(cur_widget.handle, x_var, y_var);
    }

    resolve_constraints();
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

    if (widget.image_based) {
        draw_image_container(widget);
    } else if (widget.text_based) { 
        draw_text(widget.text_info.text, glm::vec2(widget.x + widget.style.padding.x + widget.style.margin.x, widget.y - widget.style.padding.y - widget.style.margin.y), widget.text_info.text_size, widget.style.color);
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
                font_char.advance = font_char.width;
                font_char.height = 20;
                font_char.bearing.x = 5;
                font_char.bearing.y = 5;
                font_char.texture_handle = -1;
            } else {
                font_char.width = face->glyph->bitmap.width;
                font_char.advance = font_char.width * 1.025f;
                font_char.height = face->glyph->bitmap.rows;
                font_char.bearing.x = face->glyph->bitmap_left;
                font_char.bearing.y = face->glyph->bitmap_top;
                font_char.texture_handle = create_texture(face->glyph->bitmap.buffer, font_char.width, font_char.height, 0);
            }

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
	glm::mat4 projection = glm::ortho(0.0f, app.window_width, 0.0f, app.window_height);
	shader_set_mat4(data.shader, "projection", projection);
	shader_set_int(data.shader, "character_tex", 0);
	shader_set_int(data.shader, "image_tex", 1);

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
    // running_dim.height = running_dim.max_height_above_baseline + running_dim.max_height_below_baseline;
    // running_dim.height = running_dim.max_height_above_baseline;
	return running_dim;
}

void draw_background(widget_t& widget) {
	shader_set_vec3(font_char_t::ui_opengl_data.shader, "color", widget.style.background_color);
	shader_set_float(font_char_t::ui_opengl_data.shader, "tex_influence", 0.f);
	shader_set_int(font_char_t::ui_opengl_data.shader, "round_vertices", 1);
	shader_set_float(font_char_t::ui_opengl_data.shader, "border_radius", widget.style.border_radius);

    float x = widget.x + widget.style.margin.x;
    float y = widget.y - widget.style.margin.y;
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

void draw_text(const char* text, glm::vec2 starting_pos, TEXT_SIZE text_size, glm::vec3& color) {
	shader_set_vec3(font_char_t::ui_opengl_data.shader, "color", color);
	shader_set_float(font_char_t::ui_opengl_data.shader, "tex_influence", 1.f);
	shader_set_int(font_char_t::ui_opengl_data.shader, "is_character_tex", 1);
	shader_set_int(font_char_t::ui_opengl_data.shader, "round_vertices", 0);
    text_dim_t text_dim = get_text_dimensions(text, text_size);
	glm::vec2 origin = starting_pos - glm::vec2(0, text_dim.max_height_above_baseline);
	vertex_t updated_vertices[4];
    for (font_mode_t& font_mode : font_modes) {
        if (font_mode.text_size == text_size) {
            for (int i = 0; i < strlen(text); i++) {
                unsigned char c = text[i];	
                font_char_t& fc = font_mode.chars[c];

                if (fc.texture_handle != -1) {
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
                }
                origin.x += fc.advance;
            }
        }
    }
}

void draw_image_container(widget_t& widget) {
	shader_set_float(font_char_t::ui_opengl_data.shader, "tex_influence", 1.f);
	shader_set_int(font_char_t::ui_opengl_data.shader, "is_character_tex", 0);
	shader_set_int(font_char_t::ui_opengl_data.shader, "round_vertices", 0);
	vertex_t updated_vertices[4];

    bind_texture(widget.texture_handle, true);

    glm::vec2 top_left(widget.x, widget.y);

    updated_vertices[0] = create_vertex(glm::vec3(top_left.x + widget.render_width, top_left.y, 0.0f), glm::vec3(0,1,1), glm::vec2(1,1)); // top right
    updated_vertices[1] = create_vertex(glm::vec3(top_left.x + widget.render_width, top_left.y - widget.render_height, 0.f), glm::vec3(0,0,1), glm::vec2(1,0)); // bottom right
    updated_vertices[2] = create_vertex(glm::vec3(top_left.x, top_left.y - widget.render_height, 0.0f), glm::vec3(0,1,0), glm::vec2(0,0)); // bottom left
    updated_vertices[3] = create_vertex(glm::vec3(top_left.x, top_left.y, 0.0f), glm::vec3(1,0,0), glm::vec2(0,1)); // top left
    update_vbo_data(font_char_t::ui_opengl_data.vbo, (float*)updated_vertices, sizeof(updated_vertices));

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // draw the rectangle render after setting all shader parameters
    draw_obj(font_char_t::ui_opengl_data);
}