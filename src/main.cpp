
#define ADD_NETWORKING 1

#if ADD_NETWORKING == 1

#include "enet/enet.h"
#include <iostream>
#include "networking.h"
#include "app.h"
#include "renderer/renderer.h"

enum CHARACTER {
	PLAYER = 0, VILLIAN
};

struct player_info_t {
	CHARACTER character = CHARACTER::PLAYER;
	char name = 0;
};

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
	
	client_t client = create_client("brownshark123");
	server_t game_server = find_game_server(client);
	send_create_room_req(client, game_server);
	
	while (true)
    {
		process_input(mouse_state, key_state, app.window);
		if (mouse_state.left_mouse_down) {
			std::cout << "less mouse down" << std::endl;
			send_join_room_req(client, game_server);
		}
		if (mouse_state.right_mouse_down) {
			std::cout << "right mouse down" << std::endl;
		}
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
                printf("A packet of length %u containing %i was received from %s on channel %u.\n",
                       event.packet->dataLength,
                       *((int*)event.packet->data),
                       event.peer->data,
                       event.channelID);

				server_res_body_t server_res;
				memcpy(&server_res, event.packet->data, sizeof(server_res_body_t));
				std::cout << server_res.ack_type << std::endl;
				std::cout << server_res.ack_data.join_code << std::endl;

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

			render(app);
        }
    }

	enet_host_destroy(client.enet_host);

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