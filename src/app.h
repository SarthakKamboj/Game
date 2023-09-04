#pragma once

#include "SDL.h"
#include "input/input.h"
#include "shared/networking.h"
#include "networking/networking.h"
#include "fifo.h"
#include "constants.h"

struct application_t {
	bool running = true;
	SDL_Window* window = NULL;
};

enum update_mode_t {
	INTERPOLATION = 0,
	EXTRAPOLATION
};

struct update_info_t {
	update_mode_t update_mode = update_mode_t::INTERPOLATION;	
	snapshot_data_t snapshot_from;
	snapshot_data_t* snapshot_to = NULL;
};

void init(application_t& app, input_state_t& input_state);
bool assign_interpolating_snapshots(snapshots_fifo_t& snapshot_fifo, update_info_t& update_info);
void handle_snapshots(snapshots_fifo_t& snapshot_fifo, update_info_t& update_info);
void update_object_positions(update_info_t& update_info, int transform_handle);
void update(update_info_t& update_info, int transform_handle, snapshots_fifo_t& snapshot_fifo);
