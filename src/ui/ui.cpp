#include "ui.h"
#include "constants.h"

#include "renderer/basic/shape_renders.h"

std::vector<panel_t> panels;
static int panel_handle = 0;

int create_panel(float width, float height) {
    panel_t panel;
    panel.handle = panel_handle;
    panel.width = width;
    panel.height = height;
    panel_handle++;
    panels.push_back(panel);
    return panel.handle;
}

text_t create_text(const char* text, style_t& style) {
    text_t t;
    memcpy(t.text, text, fmin(sizeof(t.text), strlen(text)));
    t.styling = style;
    return t;
}

void add_text_to_panel(int panel_handle, text_t& text) {
    for (panel_t& panel : panels) {
        if (panel.handle == panel_handle) {
            panel.texts.push_back(text);
        }
    }
}

void render_ui() {
    for (panel_t& panel : panels) {
        glm::vec2 prev_anchor_point(0, panel.height);
        for (text_t& text : panel.texts) {
            text_dim_t text_dim = get_text_dimensions(text.text);
            glm::vec2 cur_render_origin = prev_anchor_point - glm::vec2(0, text_dim.height);
            if (text.styling.center_x) {
                cur_render_origin.x = (panel.width / 2) - (text_dim.width / 2);
            }
            draw_text(text.text, cur_render_origin);
            prev_anchor_point.y -= text_dim.height;
        }
    }
}

void clear_ui() {
    panels.clear();
    panel_handle = 0;    
}