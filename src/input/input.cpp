#include "input.h"
#include "constants.h"

void process_input(input_state_t& input_state) {
// void process_input(mouse_state_t& mouse_state, key_state_t& key_state, SDL_Window* window) {

	key_state_t& key_state = input_state.key_state;
	mouse_state_t& mouse_state = input_state.mouse_state;

	SDL_Event event;
    // clear data
	mouse_state.left_mouse_down = false;
	mouse_state.left_mouse_up = false;
	mouse_state.right_mouse_down = false;
	mouse_state.right_mouse_up = false;

	key_state.close_event_pressed = false;
	key_state.key_down.clear();
	key_state.key_up.clear();

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(input_state.window)) {
			key_state.close_event_pressed = true;
			continue;
		}
		switch (event.type) {
		case SDL_QUIT: {
			key_state.close_event_pressed = true;
		}
					 break;
		case SDL_MOUSEBUTTONUP: {
			if (event.button.button == SDL_BUTTON_LEFT) {
				mouse_state.left_mouse_up = true;
				mouse_state.left_mouse_being_pressed = false;
			}
			else if (event.button.button == SDL_BUTTON_RIGHT) {
				mouse_state.right_mouse_up = true;
				mouse_state.right_mouse_being_pressed = false;
			}
		}
							  break;
		case SDL_MOUSEBUTTONDOWN: {
			if (event.button.button == SDL_BUTTON_LEFT) {
				mouse_state.left_mouse_down = true;
				mouse_state.left_mouse_being_pressed = true;
			}
			else if (event.button.button == SDL_BUTTON_RIGHT) {
				mouse_state.right_mouse_down = true;
				mouse_state.right_mouse_being_pressed = true;
			}
		}
								break;
		case SDL_KEYDOWN: {
			switch (event.key.keysym.sym) {
			case SDLK_SPACE: {
				key_state.key_down[' '] = true;
				key_state.key_being_pressed[' '] = true;
				break;
			}
			case SDLK_w: {
				key_state.key_down['w'] = true;
				key_state.key_being_pressed['w'] = true;
				break;
			}
			case SDLK_a: {
				key_state.key_down['a'] = true;
				key_state.key_being_pressed['a'] = true;
				break;
			}
			case SDLK_s: {
				key_state.key_down['s'] = true;
				key_state.key_being_pressed['s'] = true;
				break;
			}
			case SDLK_d: {
				key_state.key_down['d'] = true;
				key_state.key_being_pressed['d'] = true;
				break;
			}
			default:
				break;
			}
		}
						break;
		case SDL_KEYUP: {
			switch (event.key.keysym.sym) {
			case SDLK_SPACE: {
				key_state.key_up[' '] = true;
				key_state.key_being_pressed[' '] = false;
				break;
			}
			case SDLK_w: {
				key_state.key_up['w'] = true;
				key_state.key_being_pressed['w'] = false;
				break;
			}
			case SDLK_a: {
				key_state.key_up['a'] = true;
				key_state.key_being_pressed['a'] = false;
				break;
			}
			case SDLK_s: {
				key_state.key_up['s'] = true;
				key_state.key_being_pressed['s'] = false;
				break;
			}
			case SDLK_d: {
				key_state.key_up['d'] = true;
				key_state.key_being_pressed['d'] = false;
				break;
			}
			case SDLK_ESCAPE: {
				key_state.close_event_pressed = true;
				break;
			}
			default:
				break;
			}
		}
					  break;
		default:
			break;
		}
	}

	int sdl_mouse_x, sdl_mouse_y;
	SDL_GetMouseState(&sdl_mouse_x, &sdl_mouse_y);
	mouse_state.x = sdl_mouse_x;
	mouse_state.y = WINDOW_HEIGHT - sdl_mouse_y;
}