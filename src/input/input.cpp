#if 1

#include "input.h"
#include "constants.h"
#include "SDL.h"
#include "utils/time.h"
#include "shared/networking/networking.h"
#include "networking/networking.h"

namespace input {

	void process_input(user_input_t& user_input) {

		// memset(&user_input, 0, sizeof(user_input));
		user_input.w_pressed = false;
		user_input.a_pressed = false;
		user_input.s_pressed = false;
		user_input.d_pressed = false;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYUP: {
					switch (event.key.keysym.sym) {
						case SDLK_w: {
							user_input.w_down = false;
							break;
						}
						case SDLK_a: {
							user_input.a_down = false;
							break;
						}
						case SDLK_s: {
							user_input.s_down = false;
							break;
						}
						case SDLK_d: {
							user_input.d_down = false;
							break;
						}
						default: break;
					}
				}
				break;
				case SDL_KEYDOWN: {
					switch (event.key.keysym.sym) {
						case SDLK_w: {
							user_input.w_pressed = true;
							user_input.w_down = true;
							break;
						}
						case SDLK_a: {
							user_input.a_pressed = true;
							user_input.a_down = true;
							break;
						}
						case SDLK_s: {
							user_input.s_pressed = true;
							user_input.s_down = true;
							break;
						}
						case SDLK_d: {
							user_input.d_pressed = true;
							user_input.d_down = true;
							break;
						}
						case SDLK_SPACE: {
							user_input.space_pressed = true;
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

}


#else

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
							break;
						}
						case SDLK_a: {
							user_input.a_pressed = true;
							break;
						}
						case SDLK_s: {
							user_input.s_pressed = true;
							break;
						}
						case SDLK_d: {
							user_input.d_pressed = true;
							break;
						}
						case SDLK_SPACE: {
							user_input.space_pressed = true;
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
		user_cmd.game_time = platformer::time_t::cur_time;
		user_cmd.snapshot_from_id = from_snapshot_id;
		user_cmd.snapshot_to_id = to_snapshot_id;
		user_cmd.user_input = user_input;
		user_cmd.input_cmd_id = id;
		id++;

		networking::client_cmd_t client_cmd;
		client_cmd.cmd_type = networking::CLIENT_CMD_TYPE::USER_CMD;
		client_cmd.size_of_data = sizeof(user_cmd_t);	
		client_cmd.client_cmd_data = &user_cmd;
		networking::send_client_cmd(client_cmd, false);

		user_cmds_fifo.enqueue(user_cmd);
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
			}
		}
		return cmds;
	}
}

#endif