#include "input.h"
#include "constants.h"
#include "SDL.h"
#include "utils/time.h"
// #include "shared/networking/networking.h"
// #include "networking/networking.h"

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
