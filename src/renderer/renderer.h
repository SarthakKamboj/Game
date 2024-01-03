#pragma once

// #include "basic/shape_renders.h"
#include "app.h"

struct info_pair_t {
    const char* left;
    const char* right;
};

/// <summary>
/// Render the application
/// </summary>
/// <param name="app">The application struct with the application wide settings info</param>
void render(application_t& app);
