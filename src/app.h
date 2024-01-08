#pragma once

#include "SDL.h"
#include "gameobjects/gos.h"
#include "camera.h"

#include "constants.h"

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

	float window_width = STARTING_WINDOW_WIDTH;
	float window_height = STARTING_WINDOW_HEIGHT;
	bool resized = false;
	bool is_full_screen = true;

	bool bck_muted = false;
	bool sound_fx_muted = false;

	bool new_level_just_loaded = false;

	bool running = true;
	SDL_Window* window = NULL;
	main_character_t main_character;
	camera_t camera;
	scene_manager_t scene_manager;
	bool clicked = false;

	bool paused = false;

	bool controller_state_changed = false;
	SDL_GameController* game_controller = NULL;
};

/// <summary>
/// Initializes SDL and loads the test level for development.
/// </summary>
/// <param name="app">The application info struct to be populated</param>
void init(application_t& app);

void load_level(application_t& app, int level_num);
