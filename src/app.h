#pragma once

#include "SDL.h"
#include "gameobjects/gos.h"
#include "camera.h"

// level 0 is the main menu
struct scene_manager_t {
	bool queue_level_load = false;
	int cur_level = 0;
	int level_to_load = -1;
};

void scene_manager_load_level(scene_manager_t& sm, int level_num);

/// <summary>
/// Holds application (global) level info
/// </summary>
struct application_t {
	bool running = true;
	SDL_Window* window = NULL;
	main_character_t main_character;
	camera_t camera;
	scene_manager_t scene_manager;
};

/// <summary>
/// Initializes SDL and loads the test level for development.
/// </summary>
/// <param name="app">The application info struct to be populated</param>
void init(application_t& app);

void load_level(application_t& app, int level_num);
