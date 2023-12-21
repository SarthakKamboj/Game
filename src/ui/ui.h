#pragma once

#include <vector>

// ui will be rendered immediate mode

struct style_t {
    bool center_x = false;
    bool center_y = false;
};

struct text_t {
    style_t styling;
    char text[256]{};
};
text_t create_text(const char* text, style_t& style);

struct panel_t {
    int handle = -1;
    float width = 0, height = 0;
    std::vector<text_t> texts;
};
int create_panel(float width, float height);
void add_text_to_panel(int panel_handle, text_t& text);

struct button_t {
    text_t text;
    style_t style;
};
button_t create_button(text_t& text, style_t& style);

void render_ui();
void clear_ui();