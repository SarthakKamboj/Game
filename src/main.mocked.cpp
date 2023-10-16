#if 1

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

int main(int argc, char *argv[])
{	 
	application_t app;
	init(app);

	int object_transform_handle = create_transform(glm::vec3(0), glm::vec3(1), 0);
	int obj2 = create_transform(glm::vec3(200, 500, 0), glm::vec3(1), 0);

	create_rectangle_render(object_transform_handle, glm::vec3(1,0,0), 40, 40, false, 0, -1);
	create_rectangle_render(obj2, glm::vec3(0,0,1), 40, 40, false, 0, -1);

	create_rigidbody(object_transform_handle, false, 40, 40, false);
	create_rigidbody(obj2, false, 40, 40, true);

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

#else
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
#include "shared/components/basic.h"

#define RENDER_SERVER_VIEW 1

// int object_transform_handle = -1; 
int server_object_transform_handle = -1; 
int player_transform_handle = -1;

input::user_input_t input_state;

int main(int argc, char *argv[])
{	 
	application_t app;
	init(app);

	if (networking::init_networking() != 0) {
		return EXIT_FAILURE;
	}

	int object_transform_handle = create_transform(glm::vec3(0), glm::vec3(1), 0);
	networking::add_interpolated_obj(object_transform_handle);
	int obj2 = create_transform(glm::vec3(200, 500, 0), glm::vec3(1), 0);
	networking::add_interpolated_obj(obj2);
	player_transform_handle = create_transform(glm::vec3(0), glm::vec3(1), 0);
	server_object_transform_handle = create_transform(glm::vec3(0), glm::vec3(1), 0);

	create_rectangle_render(object_transform_handle, glm::vec3(1,0,0), 40, 40, false, 0, -1);
	create_rectangle_render(obj2, glm::vec3(0,0,1), 40, 40, false, 0, -1);

#if RENDER_SERVER_VIEW
	create_rectangle_render(server_object_transform_handle, glm::vec3(0,1,0), 40, 40, false, 0, -1);
#endif
	// create_rectangle_render(player_transform_handle, glm::vec3(0,1,0), 40, 40, false, 0, -1);

	
	world::interpolated_obj_update_info_t update_info;
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
		platformer::time_t::cur_time += frame_timer.elapsed_time_sec;
		platformer::time_t::cur_server_time += frame_timer.elapsed_time_sec;
    }

	networking::cleanup_networking();

	return EXIT_SUCCESS;
}
#endif