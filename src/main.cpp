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
#include "ui/ui.h"

#define WANT_EXTRA_INFO 1

input::user_input_t input_state;
application_t app;

bool level_finished = false;

int main(int argc, char *argv[])
{	 
	// {
	// 	hash_t sha = hash("RedBlockBlue");
	// 	// printf("%016x %016x %016x %016x", sha.unsigned_double[0], sha.unsigned_double[1], sha.unsigned_double[2], sha.unsigned_double[3]);
	// 	printf("%0llx %0llx %0llx %0llx", sha.unsigned_double[0], sha.unsigned_double[1], sha.unsigned_double[2], sha.unsigned_double[3]);
	// }

	// {
	// 	hash_t sha = hash("RedBlockBlue");
	// 	printf("%0llx %0llx %0llx %0llx", sha.unsigned_double[0], sha.unsigned_double[1], sha.unsigned_double[2], sha.unsigned_double[3]);
	// }

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

	init(app);	

	std::cout << "finished init" << std::endl;

	bool paused = false;
	static uint64_t frame = 0;

	while (app.running)
    {
		utils::game_timer_t frame_timer;
		utils::start_timer(frame_timer);

		input::process_input(app, input_state);

		app.running = !input_state.quit;

		world::update(app);
		render(app);

		app.new_level_just_loaded = false;
		if (app.scene_manager.queue_level_load) {
			unload_level(app);
			clear_debug_pts();
			load_level(app, app.scene_manager.level_to_load);
			app.new_level_just_loaded = true;
		}

		utils::end_timer(frame_timer);
		if (!paused) {
			platformer::time_t::delta_time = fmin(frame_timer.elapsed_time_sec, 1 / 60.f);
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
			SDL_ClearError();
		}	

		app.resized = false;
    }

	return EXIT_SUCCESS;
}