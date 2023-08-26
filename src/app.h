#pragma once

#include "SDL.h"
// #include <map>
// #include "gameobjects/gos.h"
#include "input/input.h"

struct application_t {
	bool running = true;
	SDL_Window* window = NULL;
};

void init(application_t& app);
// void update(key_state_t& key_state, const main_character_t& mc);
