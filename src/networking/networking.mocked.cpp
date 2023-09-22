#include "networking.h"
#include "utils/time.h"
#include "update.h"
#include "transform/transform.h"

#include "shared/world/world.h"
#include "shared/input/input.h"

#include "test_config.h"

#include <cassert>

#define DROP_RATE 0.01f
#define JITTER_EFFECT 0.5f
#define NUM_SNAPSHOTS_PER_SEC 20

#ifndef RUN_TESTCASES
extern input::user_input_t input_state;
extern int server_object_transform_handle; 
#endif

namespace networking {
	float calc_new_snapshot_network_send_time(unsigned int snapshot_count) {
		// 0 to 1
		float offset = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		offset *= JITTER_EFFECT * (1.f / NUM_SNAPSHOTS_PER_SEC);
		return (static_cast<time_count_t>(snapshot_count) / NUM_SNAPSHOTS_PER_SEC) + offset;
	}

	int init_networking() {
		return 0;
	}

	client_t create_client(const char *username)
	{
		client_t client;
		client.enet_host = NULL;
		const char* test_username = "mocked_brownshark123";
		memcpy(client.username, test_username, fmin(strlen(test_username) + 1, MAX_USERNAME_LEN + 1));
		return client;
	}

	server_t find_game_server(client_t &client)
	{
		server_t server;
		server.enet_peer = NULL;
		return server;
	}

	void send_client_cmd(client_cmd_t& client_cmd, bool reliable) {}

	bool poll_network_event(network_event_t& network_event) {
#ifndef RUN_TESTCASES
		static time_count_t next_send_time = platformer::time_t::cur_server_time;

		static float x_s = 0;
		static float y_s = 50.f;

		static float g1x = 200.f;
		static float g1y = 500.f;

		static float dir[2] = { 0,0 };

		float speed = 100.f;
		// const float move = speed * platformer::time_t::delta_time;
		const float move = 0.1f;

		if (input_state.w_pressed) {
			dir[0] = 0;
			dir[1] = move;
		}
		else if (input_state.s_pressed) {
			dir[0] = 0;
			dir[1] = -move;
		}

		if (input_state.d_pressed) {
			dir[0] = move;
			dir[1] = 0;
		}
		else if (input_state.a_pressed) {
			dir[0] = -move;
			dir[1] = 0;
		}

		if (input_state.space_pressed) {
			dir[0] = 0;
			dir[1] = 0;
		}

		x_s += dir[0];
		y_s += dir[1];

		static float g1dir = 0;
		if (g1dir == 0) {
			g1x += move;
			if (g1x >= 400.f) g1dir = 1;
		}
		else {
			g1x -= move;
			if (g1x <= 100.f) g1dir = 0;
		}

		transform_t* server_transform = get_transform(server_object_transform_handle);
		server_transform->position.x = x_s;
		server_transform->position.y = y_s;

		if (platformer::time_t::cur_server_time < next_send_time) {
			network_event.event_valid = false;
			return false;
		}

		static const time_count_t time_of_first_snapshot = platformer::time_t::cur_server_time;
		static int snapshot_id = 0;

		float drop_factor = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

		if (drop_factor < DROP_RATE)
		{
			static int num_dropped = 0;
			std::cout << "dropped snapshot " << snapshot_id << std::endl;
			num_dropped++;
			snapshot_id++;
			next_send_time = time_of_first_snapshot + calc_new_snapshot_network_send_time(snapshot_id);
			network_event.event_valid = false;
			return false;
		}

		world::snapshot_t* snapshot_ptr = reinterpret_cast<world::snapshot_t*>(malloc(sizeof(world::snapshot_t)));
		assert(snapshot_ptr != NULL);
		world::snapshot_t& snapshot = *snapshot_ptr;
		snapshot.game_time = static_cast<time_count_t>(snapshot_id) / NUM_SNAPSHOTS_PER_SEC;
		snapshot.snapshot_id = snapshot_id;
		snapshot_id++;

		snapshot.gameobjects[0].x = x_s;
		snapshot.gameobjects[0].y = y_s;

		snapshot.gameobjects[1].x = g1x;
		snapshot.gameobjects[1].y = g1y;
	
		network_event.enet_event.type = ENET_EVENT_TYPE_RECEIVE;
		
		server_cmd_t server_cmd;
		server_cmd.size_of_data = sizeof(world::snapshot_t);
		server_cmd.res_type = SERVER_CMD_TYPE::SNAPSHOT;
		server_cmd.server_cmd_data = snapshot_ptr;
	
		network_event.enet_event.packet = (ENetPacket*)malloc(sizeof(ENetPacket));
		assert(network_event.enet_event.packet != NULL);
		network_event.enet_event.packet->data = reinterpret_cast<enet_uint8*>(malloc(sizeof(server_cmd_t)));
		assert(network_event.enet_event.packet->data != NULL);
		memcpy(network_event.enet_event.packet->data, &server_cmd, sizeof(server_cmd_t));

		network_event.event_valid = true;
		next_send_time = time_of_first_snapshot + calc_new_snapshot_network_send_time(snapshot_id);

		return true;
#endif
		network_event.event_valid = false;
		return false;
	}

	void destroy_network_event(network_event_t& network_event) {
		server_cmd_t* server_cmd = static_cast< server_cmd_t* >( static_cast<void*>( network_event.enet_event.packet->data ) );
		free(server_cmd->server_cmd_data);
		free(server_cmd);
		free(network_event.enet_event.packet);
	}

	void cleanup_networking() {} 

}