
#define ADD_NETWORKING 1

#if ADD_NETWORKING == 1

#include "enet/enet.h"
#include <iostream>
#include "networking/networking.h"
#include "app.h"
#include "renderer/renderer.h"
#include "utils/time.h"
#include "fifo.h"
#include "renderer/basic/shape_renders.h"
#include "constants.h"

/*
	NOTE: When an event is sent by the client, start client prediction and save between what snapshots this occurred.
	When the server gives back acknowledgement, at that moment, note the largest snapshot id in the fifo. We cannot expect
	the snapshots to display this action by the client until after that snapshot. 
*/

int main(int argc, char *argv[])
{	
	application_t app;
	input_state_t input_state;

	init(app, input_state);
	
	client_t client = create_client("brownshark123");
	server_t game_server = find_game_server(client);
	send_create_room_req(client, game_server);

	basic_network_info_t basic_frame_network_info;
	if (init_networking(basic_frame_network_info, &client, &game_server, &input_state) != 0) {
		return EXIT_FAILURE;
	}

	int transform_handle = create_transform(glm::vec3(0), glm::vec3(1), 0);
	create_rectangle_render(transform_handle, glm::vec3(1,0,0), 40, 40, false, 0, -1);
	
	update_info_t update_info;
	
	while (true)
    {
		float start = platformer::get_time_since_start_in_sec();
		process_input(input_state);
		handle_basic_frame_networking(basic_frame_network_info);

		update(update_info, transform_handle, basic_frame_network_info.snapshots_fifo);
		render(app);

		float end = platformer::get_time_since_start_in_sec();
		platformer::time_t::delta_time = end - start;
		platformer::time_t::cur_independent_time += platformer::time_t::delta_time;
    }

	cleanup_networking(basic_frame_network_info);

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