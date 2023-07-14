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

int main(int argc, char* argv[]) {
	application_t app;
	mouse_state_t mouse_state;

	init(app);

	glm::vec3 rec_color = glm::vec3(0, 1, 1);
	main_character_t mc = create_main_character(glm::vec3(WINDOW_WIDTH/2, WINDOW_HEIGHT/2, 0.f), glm::vec3(1.f), 0.f, rec_color, glm::vec2(50, 100));
 
 #if BLOCK_DEBUG
	const int NUM_BLOCKS = 15;
	ground_block_t blocks[NUM_BLOCKS + 2];
	for (int i = 0; i < NUM_BLOCKS; i++) {
		// blocks[i] = create_ground_block(glm::vec3((ground_block_t::WIDTH / 2) + ground_block_t::WIDTH * i, 20, 0.f), glm::vec3(1.f), 0.f);
	}
	float l_block_x = 150;
	float block_y = 175;
	blocks[NUM_BLOCKS] = create_ground_block(glm::vec3(l_block_x, block_y, 0.f), glm::vec3(1.f), 0.f);
	blocks[NUM_BLOCKS+1] = create_ground_block(glm::vec3(l_block_x + ground_block_t::WIDTH, block_y, 0.f), glm::vec3(1.f), 0.f);

	transform_t& transform = *get_transform(blocks[NUM_BLOCKS].transform_handle);
	int anim_handle = create_animation(&transform.position.y, block_y, block_y * 2, 3.f);
	start_animation(anim_handle);
#endif

	while (app.running) {
		float start = platformer::get_time_since_start_in_sec();
		process_input(mouse_state, key_state, app.window);	
		if (key_state.close_event_pressed) {
			app.running = false;
		}
		update(key_state, mc);
		render(app);
		float end = platformer::get_time_since_start_in_sec();
		platformer::time_t::delta_time = end - start;
	}

	return EXIT_SUCCESS;
}