#pragma once

#include <vector>
#include <unordered_map>

#include "renderer/opengl/resources.h"
#include "renderer/opengl/object_data.h"
#include "constants.h"

// ui will be rendered immediate mode

struct font_char_t {
	glm::vec2 bearing = glm::vec2(0);
	float width = 0;
	float height = 0;
	char c = 0;
	int texture_handle = -1;
	float advance = -1;

	static opengl_object_data ui_opengl_data;
};

enum class TEXT_SIZE {
    TITLE, REGULAR
};

enum class UI_PROPERTIES {
    NONE = 0,
    CLICKABLE = 1<<0,
    HOVERABLE = 1<<1
};

enum class WIDGET_SIZE {
    NONE,
    PIXEL_BASED,
    PARENT_PERCENT_BASED,
    FIT_CONTENT
};

struct font_mode_t {
    TEXT_SIZE text_size;
    int font_size = 0;
    std::unordered_map<unsigned char, font_char_t> chars;
};

struct text_dim_t {
	float width = 0;
	float height = 0;
    float max_height_above_baseline = 0;
	float max_height_below_baseline = 0;
};

void init_ui();

text_dim_t get_text_dimensions(const char* text, TEXT_SIZE text_size);

enum class DISPLAY_DIR {
    VERTICAL, HORIZONTAL
};

enum class FLOAT {
    START, CENTER, END, SPACED_OUT
};

#define TRANSPARENT_COLOR glm::vec3(-1)
glm::vec3 create_color(float r, float g, float b);

struct style_t {
    DISPLAY_DIR display_dir = DISPLAY_DIR::VERTICAL;
    FLOAT float_val = FLOAT::START;
    glm::vec2 padding = glm::vec2(0);
    float content_spacing = 0;
    glm::vec3 background_color = TRANSPARENT_COLOR;
    float border_radius = 0;
};

struct text_t {
    char text[256]{};
    TEXT_SIZE text_size = TEXT_SIZE::REGULAR;
};
void create_text(const char* text, TEXT_SIZE text_size = TEXT_SIZE::REGULAR);
bool create_button(const char* text, TEXT_SIZE text_size = TEXT_SIZE::REGULAR);

struct widget_t {
    int handle = -1;

    bool text_based = false;
    text_t text_info;

    // WIDGET_SIZE widget_size = WIDGET_SIZE::PIXEL_BASED;
    WIDGET_SIZE widget_size_width = WIDGET_SIZE::PIXEL_BASED;
    WIDGET_SIZE widget_size_height = WIDGET_SIZE::PIXEL_BASED;
    float width = -1.f;
    float height = -1.f;

    std::vector<int> children_widget_handles;
    int parent_widget_handle = NULL;

    char key[256]{};

    UI_PROPERTIES properties = UI_PROPERTIES::NONE;
    style_t style;

    // all specified in pixels with (render_x, render_y) using top left as the pt
    float render_x = -1.f;
    float render_y = -1.f;
    float render_width = -1.f;
    float render_height = -1.f;
};

void start_of_frame(bool ui_will_update = false);

void push_style(style_t& style);
void pop_style();

void push_widget(int widget_handle);
void pop_widget();

// struct container_t {
//     std::vector<text_t> texts;
//     style_t styling;
// };
// container_t create_container(style_t& style);

// struct panel_t {
//     int handle = -1;
//     const static float WIDTH;
//     const static float HEIGHT;
//     std::vector<text_t> texts;
//     std::vector<container_t> containers;
//     style_t styling;
// };

void create_panel(const char* panel_name);
void end_panel();

// void create_container(float width, float height);
// void create_container(float width, float height, WIDGET_SIZE widget_size);
void create_container(float width, float height, WIDGET_SIZE widget_size_width, WIDGET_SIZE widget_size_height);
void end_container();
// void add_text_to_panel(int panel_handle, text_t& text);

int fresh_widget_handle();
int register_widget(widget_t& widget, bool push_onto_stack = false);

struct button_t {
    text_t text;
    style_t style;
};
button_t create_button(text_t& text, style_t& style);

void autolayout_hierarchy();

struct constraint_var_t {
    int handle = -1;
    char name[256]{};
    bool constant = false;
    float* value = NULL;
    float cur_val = 0;
};

struct constraint_term_t {
    float coefficient = 0;
    int var_handle = -1;
};

struct constraint_t {
    int left_side_var_handle;
    std::vector<constraint_term_t> right_side;
    std::vector<int> constant_handles;
};

int create_constraint_var(const char* var_name, float* val);
int create_constraint_var_constant(float value);
constraint_term_t create_constraint_term(int var_handle, float coefficient);
void make_constraint_value_constant(int constraint_handle, float value);

void create_constraint(int constraint_var_handle, std::vector<constraint_term_t>& right_side_terms, float constant);
void resolve_constraints();

void draw_background(widget_t& widget);
void draw_text(const char* text, glm::vec2 starting_pos, TEXT_SIZE text_size);
void render_ui();