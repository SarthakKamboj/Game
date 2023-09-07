#pragma once

// #include <map>
// #include "SDL.h"
#include "shared/input/input.h"
#include <vector>

// struct key_state_t {
// 	std::map<unsigned char, bool> key_down;
// 	std::map<unsigned char, bool> key_up;
// 	std::map<unsigned char, bool> key_being_pressed;
// 	bool close_event_pressed = false;
// };

// struct mouse_state_t {
// 	float x = 0.f, y = 0.f;
// 	bool left_mouse_down = false, left_mouse_up = false, left_mouse_being_pressed = false;
// 	bool right_mouse_down = false, right_mouse_up = false, right_mouse_being_pressed = false;
// };

// struct input_state_t {
// 	key_state_t key_state;
// 	mouse_state_t mouse_state;
// 	SDL_Window* window = NULL;
// };

// void process_input(mouse_state_t& mouse_state, key_state_t& key_state, SDL_Window* window);
// void process_input(input_state_t& input_state);
namespace input {
	void process_input(user_input_t& user_input);
	void handle_user_cmd_networking(user_input_net_state_t& state, user_input_t& user_input);
	void send_user_cmd(user_input_t& user_input);
	user_cmd_t clear_user_cmds_upto_id(unsigned int id);
	std::vector<user_cmd_t> get_cmds_after_id(unsigned int id);
}