
#define ADD_NETWORKING 1

#if ADD_NETWORKING == 1

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

#define RENDER_SERVER_VIEW 1

/*
	NOTE: When an event is sent by the client, start client prediction and save between what snapshots this occurred.
	When the server gives back acknowledgement, at that moment, note the largest snapshot id in the fifo. We cannot expect
	the snapshots to display this action by the client until after that snapshot. 
*/

// TODO: test client times properly and is sending apropriate user cmds
// TODO: integrate server acknowledgements of user cmds on client alongside snapshots (but properly do testing of other things first)

int object_transform_handle = -1; 
int server_object_transform_handle = -1; 
int player_transform_handle = -1;

input::user_input_t input_state;

int main(int argc, char *argv[])
{	
	application_t app;

	init(app);
	
	// networking::client_t client = create_client("brownshark123");
	// networking::server_t game_server = find_game_server(client);
	// send_create_room_req(client, game_server);

	if (networking::init_networking() != 0) {
		return EXIT_FAILURE;
	}

	object_transform_handle = create_transform(glm::vec3(0), glm::vec3(1), 0);
	player_transform_handle = create_transform(glm::vec3(0), glm::vec3(1), 0);
	server_object_transform_handle = create_transform(glm::vec3(0), glm::vec3(1), 0);

	create_rectangle_render(object_transform_handle, glm::vec3(1,0,0), 40, 40, false, 0, -1);
#if RENDER_SERVER_VIEW
	create_rectangle_render(server_object_transform_handle, glm::vec3(0,1,0), 40, 40, false, 0, -1);
#endif
	// create_rectangle_render(player_transform_handle, glm::vec3(0,1,0), 40, 40, false, 0, -1);

	
	world::obj_update_info_t update_info;
	while (true)
    {
		// float start = platformer::get_time_since_start_in_sec();
		utils::game_timer_t frame_timer;
		utils::start_timer(frame_timer);

		input::process_input(input_state);
		networking::handle_network();
		// handle_basic_frame_networking(update_info, basic_frame_network_info);

		// update(input_state, update_info, object_transform_handle, player_transform_handle, basic_frame_network_info.snapshots_fifo);
		world::update(update_info);
		render(app);

		// float end = platformer::get_time_since_start_in_sec();
		utils::end_timer(frame_timer);
		// platformer::time_t::delta_time = end - start;
		// std::cout << platformer::time_t::delta_time << std::endl;
		// platformer::time_t::cur_independent_time += platformer::time_t::delta_time;
		platformer::time_t::delta_time = frame_timer.elapsed_time_sec;
		platformer::time_t::cur_independent_time += frame_timer.elapsed_time_sec;
		platformer::time_t::cur_time += frame_timer.elapsed_time_sec;
		
		// static int prev_sec_val = 0;
		// if (static_cast<int>(platformer::time_t::cur_independent_time) != prev_sec_val) {
		// 	prev_sec_val = static_cast<int>(platformer::time_t::cur_independent_time);
		// 	std::cout << platformer::time_t::cur_independent_time << std::endl;
		// }
    }

	networking::cleanup_networking();

	return EXIT_SUCCESS;
}

#else

#include "input/input.h"
#include "SDL.h"
#include "glad/glad.h"
#include "app.h"
#include <iostream>
#include "constants.h"
#include "renderer/opengl/object_data.h"
#include "renderer/renderer.h"
#include "gameobjects/gos.h"
#include "utils/time.h"
#include "animation/animation.h"

#define BLOCK_DEBUG 0

/*
Screen coordinates will always being (0,0) in the bottom left and (SCREEN_WIDTH, SCREEN_HEIGHT) in top right
*/

key_state_t key_state;

int main(int argc, char *argv[])
{
	application_t app;
	mouse_state_t mouse_state;

	init(app);

	glm::vec3 rec_color = glm::vec3(0, 1, 1);
	main_character_t mc = create_main_character(glm::vec3(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0.f), glm::vec3(1.f), 0.f, rec_color, glm::vec2(50, 100));

#if BLOCK_DEBUG
	const int NUM_BLOCKS = 15;
	ground_block_t blocks[NUM_BLOCKS + 2];
	for (int i = 0; i < NUM_BLOCKS; i++)
	{
		// blocks[i] = create_ground_block(glm::vec3((ground_block_t::WIDTH / 2) + ground_block_t::WIDTH * i, 20, 0.f), glm::vec3(1.f), 0.f);
	}
	float l_block_x = 150;
	float block_y = 175;
	blocks[NUM_BLOCKS] = create_ground_block(glm::vec3(l_block_x, block_y, 0.f), glm::vec3(1.f), 0.f);
	blocks[NUM_BLOCKS + 1] = create_ground_block(glm::vec3(l_block_x + ground_block_t::WIDTH, block_y, 0.f), glm::vec3(1.f), 0.f);

	transform_t &transform = *get_transform(blocks[NUM_BLOCKS].transform_handle);
	int anim_handle = create_animation(&transform.position.y, block_y, block_y * 2, 3.f);
	start_animation(anim_handle);
#endif

	while (app.running)
	{
		float start = platformer::get_time_since_start_in_sec();
		process_input(mouse_state, key_state, app.window);
		if (key_state.close_event_pressed)
		{
			app.running = false;
		}
		update(key_state, mc);
		render(app);
		float end = platformer::get_time_since_start_in_sec();
		platformer::time_t::delta_time = end - start;
	}

	return EXIT_SUCCESS;
}
#endif