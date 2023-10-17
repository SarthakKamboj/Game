#include "enet/enet.h"
#include <iostream>
#include "networking/networking.h"
#include "app.h"
#include "renderer/renderer.h"
#include "utils/time.h"
#include "renderer/basic/shape_renders.h"
#include "constants.h"
#include "update.h"
#include "input/input.h"

#include "shared/input/input.h"

// TODO: create many test cases for interpolation/extrapolation of interpolated objects

/*
	NOTE: When an event is sent by the client, start client prediction and save between what snapshots this occurred.
	When the server gives back acknowledgement, at that moment, note the largest snapshot id in the fifo. We cannot expect
	the snapshots to display this action by the client until after that snapshot. 
*/

// TODO: test client times properly and is sending apropriate user cmds
// TODO: integrate server acknowledgements of user cmds on client alongside snapshots (but properly do testing of other things first)

int object_transform_handle = -1; 
int player_transform_handle = -1;

int main(int argc, char *argv[])
{	
	application_t app;
	input::user_input_t input_state;

	init(app);

	if (networking::init_networking() != 0) {
		return EXIT_FAILURE;
	}

	object_transform_handle = create_transform(glm::vec3(0), glm::vec3(1), 0);
	player_transform_handle = create_transform(glm::vec3(0), glm::vec3(1), 0);

	create_rectangle_render(object_transform_handle, glm::vec3(1,0,0), 40, 40, false, 0, -1);
	create_rectangle_render(player_transform_handle, glm::vec3(0,1,0), 40, 40, false, 0, -1);
	
	world::obj_update_info_t update_info;
	while (true)
    {
		utils::game_timer_t frame_timer;
		utils::start_timer(frame_timer);

		input::process_input(input_state);
		networking::handle_network();

		world::update(update_info);
		render(app);

		utils::end_timer(frame_timer);
		platformer::time_t::delta_time = frame_timer.elapsed_time_sec;
		platformer::time_t::cur_independent_time += frame_timer.elapsed_time_sec;
		
		static int prev_sec_val = 0;
		if (static_cast<int>(platformer::time_t::cur_independent_time) != prev_sec_val) {
			prev_sec_val = static_cast<int>(platformer::time_t::cur_independent_time);
			std::cout << platformer::time_t::cur_independent_time << std::endl;
		}
    }

	networking::cleanup_networking();

	return EXIT_SUCCESS;
}