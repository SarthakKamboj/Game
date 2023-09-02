#pragma once

#include "SDL.h"
// #include <map>
// #include "gameobjects/gos.h"
#include "input/input.h"
#include "shared/networking.h"
#include "fifo.h"
#include "constants.h"

struct application_t {
	bool running = true;
	SDL_Window* window = NULL;
};

void init(application_t& app);
void update(int transform_handle, fifo<TYPE_OF_MEMBER(res_data_t, snapshot_data), MAX_SNAPSHOT_BUFFER_SIZE>& snapshot_fifo);
// void update(key_state_t& key_state, const main_character_t& mc);
