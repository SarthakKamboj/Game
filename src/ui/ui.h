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

enum UI_PROPERTIES {
    CLICKABLE = 1<<0,
    HOVERABLE = 1<<1
};

enum class WIDGET_SIZE {
    NONE,
    PIXEL_BASED
};

struct font_mode_t {
    TEXT_SIZE text_size;
    int font_size = 0;
    std::unordered_map<unsigned char, font_char_t> chars;
};

struct text_dim_t {
	float width = 0;
	float height = 0;
	float height_below_baseline = 0;
};

void init_fonts();

text_dim_t get_text_dimensions(const char* text, TEXT_SIZE text_size);
void draw_text(const char* text, glm::vec2 starting_pos, TEXT_SIZE text_size);

struct style_t {
    bool center_x = false;
    bool center_y = false;
    float content_spacing = 0;
    glm::vec2 margins = glm::vec2(0);
};

struct text_t {
    char text[256]{};
    TEXT_SIZE text_size = TEXT_SIZE::REGULAR;
};
void create_text(const char* text, TEXT_SIZE text_size = TEXT_SIZE::REGULAR);

struct widget_t {
    int handle = -1;

    bool text_based = false;
    text_t text_info;

    WIDGET_SIZE widget_size = WIDGET_SIZE::NONE;
    float width = 0.f;
    float height = 0.f;

    std::vector<int> children_widget_handles;
    int parent_widget_handle = NULL;

    char key[256]{};

    style_t style;

    // all specified in pixels with (render_x, render_y) using top left as the pt
    float render_x = 0;
    float render_y = 0;
    float render_width = 0;
    float render_height = 0;
};

void start_of_frame();

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
// void add_text_to_panel(int panel_handle, text_t& text);

int fresh_widget_handle();
void register_widget(widget_t& widget, bool push_onto_stack = false);

struct button_t {
    text_t text;
    style_t style;
};
button_t create_button(text_t& text, style_t& style);

// do research on how to do autolayout properly
void autolayout_hierarchy();
// void autolayout_hierarchy_helper(int widget_handle);

void render_ui();
void clear_ui();