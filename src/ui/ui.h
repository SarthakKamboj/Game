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
    std::vector<text_t> texts;
};
int create_panel();
void add_text_to_panel(int panel_handle, text_t& text);

struct button_t {

};

void render_ui();
void clear_ui();