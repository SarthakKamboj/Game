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
#include "physics/physics.h"

#include "shared/input/input.h"
#include "shared/components/basic.h"

#define RENDER_SERVER_VIEW 1

input::user_input_t input_state;
int object_transform_handle;

int main(int argc, char *argv[])
{	 
	application_t app;
	init(app);

	object_transform_handle = create_transform(glm::vec3(200, 500, 0), glm::vec3(1), 0);
	create_rectangle_render(object_transform_handle, glm::vec3(1,0,0), 40, 40, false, 0, -1);
	create_rigidbody(object_transform_handle, true, 40, 40, false);

	while (true)
    {
		utils::game_timer_t frame_timer;
		utils::start_timer(frame_timer);

		input::process_input(input_state);

		world::update();
		render(app);

		utils::end_timer(frame_timer);
		platformer::time_t::delta_time = frame_timer.elapsed_time_sec;
		platformer::time_t::cur_time += frame_timer.elapsed_time_sec;
		// std::cout << "fps: " << (1 / frame_timer.elapsed_time_sec) << std::endl;
    }

	return EXIT_SUCCESS;
}