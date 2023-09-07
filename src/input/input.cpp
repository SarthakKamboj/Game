#include "input.h"
#include "constants.h"
#include "SDL.h"
#include "utils/time.h"
#include "shared/networking/networking.h"
#include "networking/networking.h"

extern unsigned int from_snapshot_id;
extern unsigned int to_snapshot_id;

typedef utils::fifo<input::user_cmd_t, 30> user_cmds_fifo_t;

namespace input {

	static user_cmds_fifo_t user_cmds_fifo;

	void process_input(user_input_t& user_input) {

		memset(&user_input, 0, sizeof(user_input));

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYDOWN: {
					switch (event.key.keysym.sym) {
						case SDLK_w: {
							user_input.w_pressed = true;
							std::cout << "w pressed" << std::endl;
							break;
						}
						case SDLK_a: {
							user_input.a_pressed = true;
							std::cout << "a pressed" << std::endl;
							break;
						}
						case SDLK_s: {
							user_input.s_pressed = true;
							std::cout << "s pressed" << std::endl;
							break;
						}
						case SDLK_d: {
							user_input.d_pressed = true;
							std::cout << "d pressed" << std::endl;
							break;
						}
						case SDLK_SPACE: {
							user_input.space_pressed = true;
							std::cout << "d pressed" << std::endl;
							break;
						}
						default:
							break;
					}
				}
				break;
			}
		}
	}

	void handle_user_cmd_networking(user_input_net_state_t& state, user_input_t& user_input) {
		state.time_since_last_send += platformer::time_t::delta_time;
		const int SEND_INTERVAL = 1.0f/60.f;
		if (state.time_since_last_send >= SEND_INTERVAL) {
			state.time_since_last_send -= SEND_INTERVAL;
			send_user_cmd(user_input);
		}
	}

	void send_user_cmd(user_input_t& user_input) {
		static unsigned int id = 0;

		user_cmd_t user_cmd;
		user_cmd.game_time = platformer::time_t::cur_independent_time;
		user_cmd.snapshot_from_id = from_snapshot_id;
		user_cmd.snapshot_to_id = to_snapshot_id;
		user_cmd.user_input = user_input;
		user_cmd.input_cmd_id = id;
		id++;

		// user_input_t& user_input = user_cmd.user_input;
		// user_input.w_pressed = key_state.key_down['w'];
		// user_input.a_pressed = key_state.key_down['a'];
		// user_input.s_pressed = key_state.key_down['s'];
		// user_input.d_pressed = key_state.key_down['d'];

		networking::client_cmd_t client_cmd;
		client_cmd.cmd_type = networking::CLIENT_CMD_TYPE::USER_CMD;
		client_cmd.size_of_data = sizeof(user_cmd_t);	
		client_cmd.client_cmd_data = &user_cmd;
		networking::send_client_cmd(client_cmd, false);

		user_cmds_fifo.enqueue(user_cmd);

		// ENetPacket *packet = enet_packet_create((void *)&server_body_req, sizeof(server_body_req), 0);
		// enet_peer_send(server.enet_peer, 0, packet);
	}

	user_cmd_t clear_user_cmds_upto_id(unsigned int id) {
		user_cmds_fifo_t::peek_state_t user_cmd_peek = user_cmds_fifo.peek_read();
		while (user_cmd_peek.valid && user_cmd_peek.val->input_cmd_id != id) {
			user_cmds_fifo.dequeue();
			user_cmd_peek = user_cmds_fifo.peek_read();
		}
		if (!user_cmd_peek.valid) {
			user_cmd_t error;
			error.input_cmd_id = INVALID_INPUT_ID;
			return error;
		}
		user_cmds_fifo_t::dequeue_state_t dequeud = user_cmds_fifo.dequeue();
		return dequeud.val;
	}

	std::vector<user_cmd_t> get_cmds_after_id(unsigned int id) {
		std::vector<user_cmd_t> cmds;
		for (int i = 0; i < user_cmds_fifo.get_size(); i++) {
			user_cmds_fifo_t::peek_state_t peek = user_cmds_fifo.get_at_idx(i);
			if (peek.valid && peek.val->input_cmd_id >= id) {
				user_cmd_t& user_cmd = *peek.val;
				cmds.push_back(user_cmd);
				// user_input_t& user_input = user_cmd.user_input;
				// input_state_t input_state;
				// input_state.key_state.key_down['w'] = user_input.w_pressed;
				// input_state.key_state.key_down['a'] = user_input.a_pressed;
				// input_state.key_state.key_down['s'] = user_input.s_pressed;
				// input_state.key_state.key_down['d'] = user_input.d_pressed;
				// update_player_position(input_state, player_handle);
			}
		}
		return cmds;
	}
}

// void process_input(input_state_t& input_state) {
// // void process_input(mouse_state_t& mouse_state, key_state_t& key_state, SDL_Window* window) {

// 	key_state_t& key_state = input_state.key_state;
// 	mouse_state_t& mouse_state = input_state.mouse_state;

// 	SDL_Event event;
//     // clear data
// 	mouse_state.left_mouse_down = false;
// 	mouse_state.left_mouse_up = false;
// 	mouse_state.right_mouse_down = false;
// 	mouse_state.right_mouse_up = false;

// 	key_state.close_event_pressed = false;
// 	key_state.key_down.clear();
// 	key_state.key_up.clear();

// 	while (SDL_PollEvent(&event)) {
// 		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(input_state.window)) {
// 			key_state.close_event_pressed = true;
// 			continue;
// 		}
// 		switch (event.type) {
// 		case SDL_QUIT: {
// 			key_state.close_event_pressed = true;
// 		}
// 					 break;
// 		case SDL_MOUSEBUTTONUP: {
// 			if (event.button.button == SDL_BUTTON_LEFT) {
// 				mouse_state.left_mouse_up = true;
// 				mouse_state.left_mouse_being_pressed = false;
// 			}
// 			else if (event.button.button == SDL_BUTTON_RIGHT) {
// 				mouse_state.right_mouse_up = true;
// 				mouse_state.right_mouse_being_pressed = false;
// 			}
// 		}
// 							  break;
// 		case SDL_MOUSEBUTTONDOWN: {
// 			if (event.button.button == SDL_BUTTON_LEFT) {
// 				mouse_state.left_mouse_down = true;
// 				mouse_state.left_mouse_being_pressed = true;
// 			}
// 			else if (event.button.button == SDL_BUTTON_RIGHT) {
// 				mouse_state.right_mouse_down = true;
// 				mouse_state.right_mouse_being_pressed = true;
// 			}
// 		}
// 								break;
// 		case SDL_KEYDOWN: {
// 			switch (event.key.keysym.sym) {
// 			case SDLK_SPACE: {
// 				key_state.key_down[' '] = true;
// 				key_state.key_being_pressed[' '] = true;
// 				break;
// 			}
// 			case SDLK_w: {
// 				key_state.key_down['w'] = true;
// 				key_state.key_being_pressed['w'] = true;
// 				break;
// 			}
// 			case SDLK_a: {
// 				key_state.key_down['a'] = true;
// 				key_state.key_being_pressed['a'] = true;
// 				break;
// 			}
// 			case SDLK_s: {
// 				key_state.key_down['s'] = true;
// 				key_state.key_being_pressed['s'] = true;
// 				break;
// 			}
// 			case SDLK_d: {
// 				key_state.key_down['d'] = true;
// 				key_state.key_being_pressed['d'] = true;
// 				break;
// 			}
// 			default:
// 				break;
// 			}
// 		}
// 						break;
// 		case SDL_KEYUP: {
// 			switch (event.key.keysym.sym) {
// 			case SDLK_SPACE: {
// 				key_state.key_up[' '] = true;
// 				key_state.key_being_pressed[' '] = false;
// 				break;
// 			}
// 			case SDLK_w: {
// 				key_state.key_up['w'] = true;
// 				key_state.key_being_pressed['w'] = false;
// 				break;
// 			}
// 			case SDLK_a: {
// 				key_state.key_up['a'] = true;
// 				key_state.key_being_pressed['a'] = false;
// 				break;
// 			}
// 			case SDLK_s: {
// 				key_state.key_up['s'] = true;
// 				key_state.key_being_pressed['s'] = false;
// 				break;
// 			}
// 			case SDLK_d: {
// 				key_state.key_up['d'] = true;
// 				key_state.key_being_pressed['d'] = false;
// 				break;
// 			}
// 			case SDLK_ESCAPE: {
// 				key_state.close_event_pressed = true;
// 				break;
// 			}
// 			default:
// 				break;
// 			}
// 		}
// 					  break;
// 		default:
// 			break;
// 		}
// 	}

// 	int sdl_mouse_x, sdl_mouse_y;
// 	SDL_GetMouseState(&sdl_mouse_x, &sdl_mouse_y);
// 	mouse_state.x = sdl_mouse_x;
// 	mouse_state.y = WINDOW_HEIGHT - sdl_mouse_y;
// }