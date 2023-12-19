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

input::user_input_t input_state;

bool level_finished = false;

int main(int argc, char *argv[])
{	 
	application_t app;

	init(app);	

	bool paused = false;

	while (!input_state.quit)
    {
		utils::game_timer_t frame_timer;
		utils::start_timer(frame_timer);

		input::process_input(input_state);

		if (input_state.p_pressed) {
			paused = !paused;
		}

		world::update(app);
		render(app);

		if (app.scene_manager.queue_level_load) {
			app.scene_manager.queue_level_load = false;
			unload_level(app);
			clear_debug_pts();
			load_level(app, app.scene_manager.level_to_load);
			app.scene_manager.level_to_load = -1;
		}

		utils::end_timer(frame_timer);
		if (!paused) {
			platformer::time_t::delta_time = frame_timer.elapsed_time_sec;
			static unsigned int i = 0;
			i++;
			if (i % 1000000) {
				// std::cout << "fps: " << (1 / frame_timer.elapsed_time_sec) << std::endl;
			}
		}
		else {
			platformer::time_t::delta_time = 0;
		}
		platformer::time_t::cur_time += platformer::time_t::delta_time;	
    }

	return EXIT_SUCCESS;
}