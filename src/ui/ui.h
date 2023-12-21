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
    TEXT_SIZE text_size = TEXT_SIZE::REGULAR;
    glm::vec2 dims = glm::vec2(0);
    glm::vec2 margins = glm::vec2(0);
};

struct text_t {
    style_t styling;
    char text[256]{};
};
text_t create_text(const char* text, style_t& style);

struct container_t {
    std::vector<text_t> texts;
    style_t styling;
};
container_t create_container(style_t& style);

struct panel_t {
    int handle = -1;
    const static float WIDTH;
    const static float HEIGHT;
    std::vector<text_t> texts;
    std::vector<container_t> containers;
    style_t styling;
};

int create_panel(style_t& styling);
void add_text_to_panel(int panel_handle, text_t& text);

struct button_t {
    text_t text;
    style_t style;
};
button_t create_button(text_t& text, style_t& style);

void render_ui();
void clear_ui();