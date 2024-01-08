#pragma once

// #include "basic/shape_renders.h"
#include "app.h"

struct info_pair_t {
    const char* left;
    const char* right;
};

struct bool_settings_state_t {
    bool bck_muted = false;
    bool sound_fx_muted = false;
    bool is_full_screen = false;
};

struct settings_changed_t {
    bool aspect_ratio = false;
    bool bck_music = false;
    bool sound_fx = false;
    bool windowed = false;
};
bool something_changed(settings_changed_t settings_changed);

// enum class ASPECT_RATIO {
//     A_1920x1080,
//     A_1600x900,
//     A_1440x990,
//     A_1366x768,
//     A_1280x1024,
// };

enum ASPECT_RATIO {
    A_1600x900 = 0,
    A_1600x1000,
    A_1600x1200,

    NUM_RATIOS,

    A_2100x900,
    A_800x800,

};

struct aspect_ratio_t {
    ASPECT_RATIO ratio;
    const char* str;
    float width = 0.f;
    float height = 0.f;
    int mode_index = -1;
};

/// <summary>
/// Render the application
/// </summary>
/// <param name="app">The application struct with the application wide settings info</param>
void render(application_t& app);
