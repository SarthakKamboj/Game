#pragma once

#if 1

#include "SDL.h"

/// <summary>
/// Holds application (global) level info
/// </summary>
struct application_t {
	bool running = true;
	SDL_Window* window = NULL;
};

/// <summary>
/// Initializes SDL and loads the test level for development.
/// </summary>
/// <param name="app">The application info struct to be populated</param>
void init(application_t& app);

#else

#include "SDL.h"
#include "input/input.h"
#include "shared/networking/networking.h"
#include "networking/networking.h"
#include "shared/fifo.h"
#include "constants.h"

struct application_t {
	bool running = true;
	SDL_Window* window = NULL;
};

enum object_update_mode_t {
	INTERPOLATION = 0,
	EXTRAPOLATION
};

enum player_update_mode_t {
	INTERPOLATION = 0,
	PREDICTION
};

struct obj_update_info_t {
	object_update_mode_t update_mode = object_update_mode_t::INTERPOLATION;	
	snapshot_t snapshot_from;
	snapshot_t* snapshot_to = NULL;
	time_count_t cur_time = 0;
};

void init(application_t& app, input_state_t& input_state);
bool assign_interpolating_snapshots(snapshots_fifo_t& snapshot_fifo, obj_update_info_t& update_info);
void handle_snapshots(snapshots_fifo_t& snapshot_fifo, obj_update_info_t& update_info);
void update_object_positions(obj_update_info_t& update_info, int transform_handle);
void update_player_position(input_state_t& input_state, int player_transform_handle);
void update(input_state_t& input_state, obj_update_info_t& update_info, int transform_handle, int player_transform_handle, snapshots_fifo_t& snapshot_fifo);

#endif