
#define ADD_NETWORKING 1

#if ADD_NETWORKING == 1

#include "enet/enet.h"
#include <iostream>
#include "networking.h"
#include "app.h"
#include "renderer/renderer.h"
#include "utils/time.h"
#include "fifo.h"
#include "renderer/basic/shape_renders.h"
#include "constants.h"

#define SNAPSHOT_UNIT_TEST 1

fifo<TYPE_OF_MEMBER(res_data_t, snapshot_data), MAX_SNAPSHOT_BUFFER_SIZE> gameobject_saved_snapshots;

float calc_new_time_between_snapshots(const int NUM_SNAPSHOTS_PER_SEC) {
	float offset = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	offset -= 1.f;
	offset *= (1.f / NUM_SNAPSHOTS_PER_SEC) * 0.5f;
	return (1.f / NUM_SNAPSHOTS_PER_SEC) + offset;
}

int main(int argc, char *argv[])
{
	int enet_init = enet_initialize();
	if (enet_init != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}

	std::string s = "enet successfully initialized";

	application_t app;
	key_state_t key_state;
	mouse_state_t mouse_state;

	init(app);
	
#if SNAPSHOT_UNIT_TEST == 0
	client_t client = create_client("brownshark123");
	server_t game_server = find_game_server(client);
	send_create_room_req(client, game_server);
#endif

	int transform_handle = create_transform(glm::vec3(0), glm::vec3(1), 0);
	create_rectangle_render(transform_handle, glm::vec3(1,0,0), 40, 40, false, 0, -1);
	
	while (true)
    {
		float start = platformer::get_time_since_start_in_sec();
		process_input(mouse_state, key_state, app.window);
		static float time_since_last_send = 0.f;
		time_since_last_send += platformer::time_t::delta_time;
		const int SEND_INTERVAL = 1.0f/60.f;
		if (time_since_last_send >= SEND_INTERVAL) {
			time_since_last_send -= SEND_INTERVAL;
#if SNAPSHOT_UNIT_TEST == 0
			send_user_cmd(client, game_server, key_state);
#endif
		}
		if (mouse_state.left_mouse_down) {
			std::cout << "left mouse down" << std::endl;
		}
		if (mouse_state.right_mouse_down) {
			std::cout << "right mouse down" << std::endl;
		}
		      
#if SNAPSHOT_UNIT_TEST == 0
        ENetEvent event;
        while (enet_host_service(client.enet_host, &event, 0) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT: {
                char hostname[256]{};
                enet_address_get_host_ip(&event.peer->address, hostname, sizeof(hostname));
                std::cout << "A newclient connected from " << hostname << ":" << event.peer->address.port << std::endl;
            }
                break;
            case ENET_EVENT_TYPE_RECEIVE: {
                /* printf("A packet of length %u containing %i was received from %s on channel %u.\n",
                       event.packet->dataLength,
                       *((int*)event.packet->data),
                       event.peer->data,
                       event.channelID); */

				server_res_body_t server_res;
				memcpy(&server_res, event.packet->data, sizeof(server_res_body_t));
				std::cout << server_res.res_type << std::endl;
				if (server_res.res_type == RES_TYPE::JOIN_CODE) {
					std::cout << server_res.res_data.join_code << std::endl;
				} else if (server_res.res_type == RES_TYPE::RES_MSG) {
					std::cout << server_res.res_data.msg << std::endl;
				}
				else if (server_res.res_type == RES_TYPE::SNAPSHOT) {
					static unsigned int last_enqueued_snapshot_id = 0;
					snapshot_data_t& snapshot = server_res.res_data.snapshot_data;

					gameobject_snapshot_t& game_object = snapshot.gameobject_snapshots[0];
					// std::cout << "snapshot received with x: " << game_object.x << " and y: " << game_object.y << std::endl;

					gameobject_saved_snapshots.enqueue(snapshot);

					// if (snapshot.snapshot_id - last_enqueued_snapshot_id == 1) {
					// } else if (snapshot.snapshot_id - last_enqueued_snapshot_id == 2) {
					// 	gameobject_saved_snapshots.enqueue(snapshot);
					// }
				}

                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy(event.packet);
            }
                break;

            case ENET_EVENT_TYPE_DISCONNECT: {
                printf("%s disconnected.\n", event.peer->data);
                /* Reset the peer's client information. */
                event.peer->data = NULL;
            }
                break;

            default: {
                printf("event type not handled");
            }
                break;
            }

        }
#else

		const int NUM_SNAPSHOTS_PER_SEC = 20;
		float TIME_BETWEEN_SNAPSHOTS = 1.f / NUM_SNAPSHOTS_PER_SEC;
		static float time_since_last_receive = TIME_BETWEEN_SNAPSHOTS;
		time_since_last_receive += platformer::time_t::delta_time;
		if (time_since_last_receive >= TIME_BETWEEN_SNAPSHOTS) {
			static int snapshot_id = 0;
			static float game_time = -TIME_BETWEEN_SNAPSHOTS;
			static float running_time_between = 0.f;
			static int last_frame_prev_size = 0;

			game_time += TIME_BETWEEN_SNAPSHOTS;
			running_time_between += TIME_BETWEEN_SNAPSHOTS;

			// std::cout << "game_time: " << game_time << std::endl;

			float skip = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

			if (skip < 0.0125f) {
				snapshot_id++;
				std::cout << "skipped " << snapshot_id << std::endl;
				time_since_last_receive = 0.f;

			} else {
				time_since_last_receive -= TIME_BETWEEN_SNAPSHOTS;

				fifo<snapshot_data_t, MAX_SNAPSHOT_BUFFER_SIZE>::peek_state_t peek_state = gameobject_saved_snapshots.peek_read();
				// snapshot receiving out of order logic
				if (peek_state.valid && snapshot_id < peek_state.val->snapshot_id) {
					std::cout << "cannot render this because it is too far back in interpolation time on client" << std::endl;
				} else {
					snapshot_data_t snapshot{};
					snapshot.snapshot_id = snapshot_id;
					snapshot_id++;
					snapshot.game_time = game_time;
					// snapshot.gameobject_snapshots[0].x = snapshot_id * 10;

					static float x = 0.f;
					static float y = 0.f;

					float speed = 10.0f;

					if (key_state.key_being_pressed['d']) {
						x += speed;
					} else if (key_state.key_being_pressed['a']) {
						x -= speed;
					}

					if (key_state.key_being_pressed['w']) {
						y += speed;
					} else if (key_state.key_being_pressed['s']) {
						y -= speed;
					}

					// snapshot.gameobject_snapshots[0].x = WINDOW_WIDTH * static_cast<float>(rand()) / static_cast<float> (RAND_MAX);
					// snapshot.gameobject_snapshots[0].y = WINDOW_HEIGHT * static_cast<float>(rand()) / static_cast<float> (RAND_MAX);

					snapshot.gameobject_snapshots[0].x = x;
					snapshot.gameobject_snapshots[0].y = y;

					// int prev_size = gameobject_saved_snapshots.get_size();
					bool enqueued = gameobject_saved_snapshots.enqueue(snapshot);	

					if (!enqueued) {
						static int num_dropped = 0;
						num_dropped++;
						std::cout << num_dropped << " snapshots had to be dropped b/c fifo too full" << std::endl;
					} else {
						int new_size = gameobject_saved_snapshots.get_size();

						// if (new_size > 3 && new_size != last_frame_prev_size) {
						// 	std::cout << "size has grown beyond necessary" << std::endl;
						// }
						gameobject_snapshot_t& gs = snapshot.gameobject_snapshots[0];
						// std::cout << "queued snapshot " << snapshot.snapshot_id << " at position x: " << gs.x << " and at position y: " << gs.y << std::endl;
					}
					// std::cout << "took " << TIME_BETWEEN_SNAPSHOTS << " seconds to get this snapshot" << std::endl;

				}

				TIME_BETWEEN_SNAPSHOTS = calc_new_time_between_snapshots(NUM_SNAPSHOTS_PER_SEC);

				if (TIME_BETWEEN_SNAPSHOTS < 0) {
					std::cout << "TIME_BETWEEN_SNAPSHOTS is less than 0" << std::endl;
				}

				last_frame_prev_size = gameobject_saved_snapshots.get_size();
			}
		}
		
#endif

		update(transform_handle, gameobject_saved_snapshots);
		render(app);
		float end = platformer::get_time_since_start_in_sec();
		platformer::time_t::delta_time = end - start;
    }

#if SNAPSHOT_UNIT_TEST == 0
	enet_host_destroy(client.enet_host);
#endif

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