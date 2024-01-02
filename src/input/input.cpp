#include "input.h"
#include "constants.h"
#include "SDL.h"
#include "utils/time.h"
#include "app.h"

#include <iostream>

namespace input {

    void init_controller(application_t& app) {
		for (int i = 0; i < SDL_NumJoysticks(); i++) {
			if (SDL_IsGameController(i)) {
				app.game_controller = SDL_GameControllerOpen(i);
				std::cout << "found game controller joystick " << i << std::endl;
			}
		}
	}

	void process_input(application_t& app, user_input_t& user_input) {

		SDL_GetMouseState(&user_input.x_pos, &user_input.y_pos);
		// user_input.y_pos = WINDOW_HEIGHT - user_input.y_pos;
		user_input.y_pos = app.window_height - user_input.y_pos;

		user_input.some_key_pressed = false;
		user_input.w_pressed = false;
		user_input.a_pressed = false;
		user_input.s_pressed = false;
		user_input.d_pressed = false;
		user_input.p_pressed = false;
		user_input.l_pressed = false;
		user_input.space_pressed = false;

		user_input.controller_a_pressed = false;
		user_input.controller_y_pressed = false;

		user_input.quit = false;
		user_input.left_clicked = false;
		user_input.right_clicked = false;

		user_input.controller_x_axis = 0;
		user_input.controller_y_axis = 0;

		if (app.game_controller) {
			user_input.controller_x_axis = SDL_GameControllerGetAxis(app.game_controller, SDL_CONTROLLER_AXIS_LEFTX) / 32768.f;
			user_input.controller_y_axis = SDL_GameControllerGetAxis(app.game_controller, SDL_CONTROLLER_AXIS_LEFTY) / 32768.f;
		}

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: {
					user_input.quit = true;
					break;
				}
				case SDL_WINDOWEVENT: {
					switch (event.window.event) {
						case SDL_WINDOWEVENT_MOVED: {
							int x = event.window.data1;
							int y = event.window.data2;
							break;
						}
						// seems like this is called 2x on resizes
						case SDL_WINDOWEVENT_RESIZED: {
							break;
						}
						// seems like this is called 1x on resizes
						case SDL_WINDOWEVENT_SIZE_CHANGED: {
							app.window_width = event.window.data1;
							app.window_height = event.window.data2;
							app.resized = true;
							glViewport(0, 0, app.window_width, app.window_height);
							break;
						}
						default: break;
					}
					break;
				}
				case SDL_CONTROLLERDEVICEADDED: {
					if (!app.game_controller) {
						init_controller(app);
					}
					break;
				}
				case SDL_CONTROLLERDEVICEREMOVED: {
					if (app.game_controller) {
						SDL_Joystick* controller_joystick = SDL_GameControllerGetJoystick(app.game_controller);
						if (event.cdevice.which == SDL_JoystickInstanceID(controller_joystick)) {
							app.game_controller = NULL;
							user_input.controller_a_down = false;
							user_input.controller_a_pressed = false;
							user_input.controller_y_down = false;
							user_input.controller_y_pressed = false;
							user_input.controller_x_axis = 0;
							user_input.controller_y_axis = 0;
							init_controller(app);
						}
					}
					break;
				}
				case SDL_CONTROLLERBUTTONUP: {
					if (app.game_controller) {
						SDL_Joystick* controller_joystick = SDL_GameControllerGetJoystick(app.game_controller);
						if (event.cdevice.which == SDL_JoystickInstanceID(controller_joystick)) {
							switch (event.cbutton.button) {
								case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A: {
									user_input.controller_a_down = false;
									break;
								}
								case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y: {
									user_input.controller_y_down = false;
									break;
								}
								default: break;
							}
						}
					}
					break;	
				}

				case SDL_CONTROLLERBUTTONDOWN: {
					if (app.game_controller) {
						SDL_Joystick* controller_joystick = SDL_GameControllerGetJoystick(app.game_controller);
						if (event.cdevice.which == SDL_JoystickInstanceID(controller_joystick)) {
							switch (event.cbutton.button) {
								case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A: {
									user_input.controller_a_pressed = true;
									user_input.controller_a_down = true;
									break;
								}
								case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y: {
									user_input.controller_y_pressed = true;
									user_input.controller_y_down = true;
									break;
								}
								default: break;
							}
						}
					}
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
					user_input.some_key_pressed = true;
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
