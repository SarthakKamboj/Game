#include "input.h"
#include "constants.h"
#include "SDL.h"
#include "utils/time.h"

#include <iostream>

namespace input {

	void process_input(user_input_t& user_input) {

		SDL_GetMouseState(&user_input.x_pos, &user_input.y_pos);
		user_input.y_pos = WINDOW_HEIGHT - user_input.y_pos;

		user_input.w_pressed = false;
		user_input.a_pressed = false;
		user_input.s_pressed = false;
		user_input.d_pressed = false;
		user_input.p_pressed = false;
		user_input.l_pressed = false;
		user_input.space_pressed = false;
		user_input.quit = false;
		user_input.left_clicked = false;
		user_input.right_clicked = false;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: {
					user_input.quit = true;
					break;
				}
				case SDL_MOUSEBUTTONUP: {
					user_input.right_clicked = (event.button.button == SDL_BUTTON_RIGHT);
					user_input.left_clicked = (event.button.button == SDL_BUTTON_LEFT);	
				}
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
						case SDLK_p: {
							user_input.p_down = false;
							break;
						}
						case SDLK_l: {
							user_input.l_down = false;
							break;
						}
						default: break;
					}
				}
				break;
				case SDL_KEYDOWN: {
					switch (event.key.keysym.sym) {
						case SDLK_ESCAPE: {
							user_input.quit = true;
							break;
						}
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
						case SDLK_l: {
							user_input.l_pressed = true;
							user_input.l_down = true;
							break;
						}
						case SDLK_SPACE: {
							user_input.space_pressed = true;
							break;
						}
						case SDLK_p: {
							user_input.p_pressed = true;
							user_input.p_down = true;
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
