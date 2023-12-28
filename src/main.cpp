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

input::user_input_t input_state;

bool level_finished = false;

int main(int argc, char *argv[])
{	 
	application_t app;

	init(app);	

	bool paused = false;

	while (app.running)
    {
		try {
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

			if (input_state.p_pressed) {
				paused = !paused;
			}

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

			const char* sdlError = SDL_GetError();
			if (sdlError && sdlError[0] != '\0') {
				std::cerr << "SDL Error: " << sdlError << std::endl;
			}	

		} catch (const std::exception& e) {
            printf("error in world update");
            std::cout << e.what();
			exit(-1);
        }
    }

	return EXIT_SUCCESS;
}