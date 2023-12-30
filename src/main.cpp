#include <iostream>
#include "app.h"
#include "renderer/renderer.h"
#include "utils/time.h"
#include "renderer/basic/shape_renders.h"
#include "constants.h"
#include "update.h"
#include "input/input.h"
#include "physics/physics.h"
#include "animation/animation.h"
#include "SDL.h"
#include "audio/audio.h"
#include "utils/io.h"

#define WANT_EXTRA_INFO 1

input::user_input_t input_state;

bool level_finished = false;

int main(int argc, char *argv[])
{	 
	bool running_in_vs = false;
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			if (strcmp("RUN_FROM_IDE", argv[i]) == 0) {
				running_in_vs = true;
			}
		}
	}

	io::set_running_in_visual_studio(running_in_vs);

#if WANT_EXTRA_INFO
	std::cout << "running_in_vs: " << running_in_vs << std::endl;

	char buffer[256]{};
	io::get_resources_folder_path(buffer);
	std::cout << buffer << std::endl;
#endif

	application_t app;

	init(app);	

	std::cout << "finished init" << std::endl;

	bool paused = false;
	static uint64_t frame = 0;

	while (app.running)
    {
		utils::game_timer_t frame_timer;
		utils::start_timer(frame_timer);

		input::process_input(input_state);

		// static bool pause = true;
		// if (input_state.left_clicked) {
		// 	if (pause) {
		// 		pause_bck_sound();
		// 	} else {
		// 		resume_bck_sound();
		// 	}
		// 	pause = !pause;
		// }

		app.running = !input_state.quit;

		// if (input_state.p_pressed) {
		// 	paused = !paused;
		// }

		world::update(app);
		render(app);

		if (app.scene_manager.queue_level_load) {
			unload_level(app);
			clear_debug_pts();
			load_level(app, app.scene_manager.level_to_load);
		}

		utils::end_timer(frame_timer);
		if (!paused) {
			platformer::time_t::delta_time = frame_timer.elapsed_time_sec;
			static unsigned int i = 0;
			i++;
		}
		else {
			platformer::time_t::delta_time = 0;
		}
		platformer::time_t::cur_time += platformer::time_t::delta_time;	

		if (platformer::time_t::cur_time > 5) {
			app.clicked = true;
		}

		// static int i = 0;
		// i++;

		if (input_state.p_pressed) {
			std::cout << "fps: " << 1 / platformer::time_t::delta_time << std::endl;
		}

		const char* sdlError = SDL_GetError();
		if (sdlError && sdlError[0] != '\0') {
			std::cout << "SDL Error: " << sdlError << std::endl;
		}	
    }

	return EXIT_SUCCESS;
}