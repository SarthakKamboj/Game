#pragma once

#include <map>
#include "SDL.h"

struct key_state_t {
	std::map<unsigned char, bool> key_down;
	std::map<unsigned char, bool> key_up;
	std::map<unsigned char, bool> key_being_pressed;
	bool close_event_pressed = false;
};

struct mouse_state_t {
	float x = 0.f, y = 0.f;
	bool left_mouse_down = false, left_mouse_up = false, left_mouse_being_pressed = false;
	bool right_mouse_down = false, right_mouse_up = false, right_mouse_being_pressed = false;
};

void process_input(mouse_state_t& mouse_state, key_state_t& key_state, SDL_Window* window);
