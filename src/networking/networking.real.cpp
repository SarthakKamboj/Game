#if 1

#include <cassert>

#include "networking.h"
#include "update.h"

namespace networking {
	static network_state_t network_state;

	int init_networking() {
		int enet_init = enet_initialize();
		if (enet_init != 0) {
			fprintf(stderr, "An error occurred while initializing ENet.\n");
			return enet_init;
		}	

		network_state.client = networking::create_client("brownshark123");
		network_state.server = networking::find_game_server(network_state.client);
	}

	bool poll_network_event(network_event_t& network_event) {
		network_event.event_valid = enet_host_service(network_state.client.enet_host, &network_event.enet_event, 0) > 0;
		return network_event.event_valid;
	}

	client_t create_client(const char *username)
	{
		client_t client;
		ENetHost *enet_host = enet_host_create(NULL, 1, 2, 0, 0);
		assert(enet_host != NULL);
		client.enet_host = enet_host;
		memcpy(client.username, username, fmin(strlen(username) + 1, MAX_USERNAME_LEN + 1));
		return client;
	}

	server_t find_game_server(client_t &client)
	{
		server_t server;

		ENetAddress address;
		enet_address_set_host(&address, SERVER_IP);
		address.port = SERVER_PORT;

		ENetPeer *peer = enet_host_connect(client.enet_host, &address, 2, 0);
		assert(peer != NULL);

		ENetEvent conn_event;
		int num_conn_evnts = enet_host_service(client.enet_host, &conn_event, 5000);
		if (num_conn_evnts > 0)
		{
			if (conn_event.type == ENET_EVENT_TYPE_CONNECT)
			{
				char hostname[256]{};
				enet_address_get_host_ip(&conn_event.peer->address, hostname, sizeof(hostname));
				printf("A new client connected from %s:%u.\n",
					hostname,
					conn_event.peer->address.port);

				server.enet_peer = peer;
				enet_host_flush(client.enet_host);
			}
			else if (conn_event.type == ENET_EVENT_TYPE_DISCONNECT)
			{
				printf("Tried connecting to %s:%i, but failed.", SERVER_IP, SERVER_PORT);
			}
		}
		else
		{
			enet_peer_reset(peer);
			printf("Connection to %s:%i failed.", SERVER_IP, SERVER_PORT);
		}

		return server;
	}

	void send_client_cmd(client_cmd_t& client_cmd, bool reliable) {
		unsigned int alloc_size = sizeof(client_cmd_t) - sizeof(client_cmd_data_t) + client_cmd.size_of_data;
		void* alloc_ptr = malloc(alloc_size);
		unsigned int* data_size_ptr = reinterpret_cast<unsigned int*>(alloc_ptr);
		*data_size_ptr = client_cmd.size_of_data;
		data_size_ptr++;
		CLIENT_CMD_TYPE* cmd_type_ptr = reinterpret_cast<CLIENT_CMD_TYPE*>(data_size_ptr);
		*cmd_type_ptr = client_cmd.cmd_type;
		cmd_type_ptr++;
		char* data_ptr = reinterpret_cast<char*>(cmd_type_ptr);
		memcpy(data_ptr, client_cmd.client_cmd_data, client_cmd.size_of_data);
		ENetPacket* packet = NULL;
		if (reliable) {
			packet = enet_packet_create(alloc_ptr, alloc_size, ENET_PACKET_FLAG_RELIABLE);
		} else {
			packet = enet_packet_create(alloc_ptr, alloc_size, 0);
		}
		enet_peer_send(network_state.server.enet_peer, 0, packet);
	}

    void destroy_network_event(network_event_t& network_event) {
		enet_packet_destroy(network_event.enet_event.packet);
	}

	void cleanup_networking() {
		enet_host_destroy(network_state.client.enet_host);
	}
}

#else
#include "networking.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <math.h>
#include "utils/time.h"
#include "shared/utils/fifo.h"
#include "shared/networking/networking.h"
#include "constants.h"
#include "app.h"
#include "transform/transform.h"

static user_cmds_fifo_t user_cmds_fifo;

int init_networking(basic_network_info_t& basic_networking_info, client_t* client, server_t* server, input_state_t* input_state) {
	int enet_init = enet_initialize();
	if (enet_init != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return enet_init;
	}

	std::string s = "enet successfully initialized";
	basic_networking_info.client = client;	
	basic_networking_info.server = server;	
	basic_networking_info.input_state = input_state;	
	basic_networking_info.time_since_last_user_cmd_send = 0.f;
}

void cleanup_networking(basic_network_info_t& basic_networking_info) {
	enet_host_destroy(basic_networking_info.client->enet_host);
}

client_t create_client(const char *username)
{
	client_t client;
	ENetHost *enet_host = enet_host_create(NULL, 1, 2, 0, 0);
	assert(enet_host != NULL);
	client.enet_host = enet_host;
	memcpy(client.username, username, fmin(strlen(username) + 1, MAX_USERNAME_LEN + 1));
	return client;
}

void send_create_room_req(client_t &client, server_t &server)
{
	server_req_body_t server_req;
	server_req.req_type = REQ_TYPE::CREATE_ROOM;
	memcpy(server_req.req_data.username, client.username, strlen(client.username) + 1);

	ENetPacket *packet = enet_packet_create((void *)&server_req, sizeof(server_req), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(server.enet_peer, 0, packet);

	enet_host_flush(client.enet_host);
}

void send_join_room_req(client_t &client, server_t &server)
{
	server_req_body_t server_req;
	server_req.req_type = REQ_TYPE::JOIN_ROOM;
	memcpy(server_req.req_data.join_room_info.username, client.username, strlen(client.username) + 1);
	server_req.req_data.join_room_info.room_join_code = 10245;

	ENetPacket *packet = enet_packet_create((void *)&server_req, sizeof(server_req), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(server.enet_peer, 0, packet);

	enet_host_flush(client.enet_host);
}

void handle_basic_frame_networking(obj_update_info_t& update_info, basic_network_info_t& frame_basic_network_info) {
	assert(frame_basic_network_info.client != NULL);
	assert(frame_basic_network_info.server != NULL);
	assert(frame_basic_network_info.input_state != NULL);

	client_t& client = *frame_basic_network_info.client;
	server_t& server = *frame_basic_network_info.server;
	input_state_t& input_state = *frame_basic_network_info.input_state;
	handle_user_cmd_networking(update_info, frame_basic_network_info);
	network_event_t server_event;
	while (poll_server_event(client, server_event)) {
		handle_server_event(frame_basic_network_info, server_event);
	}
}

bool poll_server_event(client_t& client, network_event_t& network_event) {
	network_event.event_valid = enet_host_service(client.enet_host, &network_event.enet_event, 0) > 0;
	return network_event.event_valid;
}

void handle_user_cmd_networking(obj_update_info_t& update_info, basic_network_info_t& basic_info) {
	basic_info.time_since_last_user_cmd_send += platformer::time_t::delta_time;
	const int SEND_INTERVAL = 1.0f/60.f;
	if (basic_info.time_since_last_user_cmd_send >= SEND_INTERVAL) {
		basic_info.time_since_last_user_cmd_send -= SEND_INTERVAL;
		send_user_cmd(update_info, *basic_info.client, *basic_info.server, basic_info.input_state->key_state);
	}
}

server_t find_game_server(client_t &client)
{
	server_t server;

	ENetAddress address;
	enet_address_set_host(&address, SERVER_IP);
	address.port = SERVER_PORT;

	ENetPeer *peer = enet_host_connect(client.enet_host, &address, 2, 0);
	assert(peer != NULL);

	ENetEvent conn_event;
	int num_conn_evnts = enet_host_service(client.enet_host, &conn_event, 5000);
	if (num_conn_evnts > 0)
	{
		if (conn_event.type == ENET_EVENT_TYPE_CONNECT)
		{
			char hostname[256]{};
			enet_address_get_host_ip(&conn_event.peer->address, hostname, sizeof(hostname));
			printf("A new client connected from %s:%u.\n",
				   hostname,
				   conn_event.peer->address.port);

			server.enet_peer = peer;

			// TODO: see if this flush is necessary
			enet_host_flush(client.enet_host);
		}
		else if (conn_event.type == ENET_EVENT_TYPE_DISCONNECT)
		{
			printf("Tried connecting to %s:%i, but failed.", SERVER_IP, SERVER_PORT);
		}
	}
	else
	{
		enet_peer_reset(peer);
		printf("Connection to %s:%i failed.", SERVER_IP, SERVER_PORT);
	}

	return server;
}

void send_user_cmd(obj_update_info_t& update_info, client_t &client, server_t &server, key_state_t &key_state)
{
	static unsigned int id = 0;
	server_req_body_t server_body_req;
	server_body_req.req_type = REQ_TYPE::USER_CMD;

	auto &user_cmd = server_body_req.req_data.user_cmd_info;
	// user_cmd.game_time = running_time;
	user_cmd.game_time = platformer::time_t::cur_independent_time;
	user_cmd.gameobject_updating = GAMEOBJECT_UPDATING::PLAYER;
	user_cmd.snapshot_from_id = update_info.snapshot_from.snapshot_id;
	user_cmd.snapshot_to_id = update_info.snapshot_to->snapshot_id;
	user_cmd.input_cmd_id = id;
	id++;
	
	user_input_t& user_input = user_cmd.user_input;
	user_input.w_pressed = key_state.key_down['w'];
	user_input.a_pressed = key_state.key_down['a'];
	user_input.s_pressed = key_state.key_down['s'];
	user_input.d_pressed = key_state.key_down['d'];

	user_cmds_fifo.enqueue(user_cmd);

	ENetPacket *packet = enet_packet_create((void *)&server_body_req, sizeof(server_body_req), 0);
	enet_peer_send(server.enet_peer, 0, packet);
}

void handle_server_event(int player_transform, basic_network_info_t& network_info, network_event_t& network_event)
{
	if (network_event.event_valid) return;
	ENetEvent& event = network_event.enet_event;
	switch (event.type)
	{
		case ENET_EVENT_TYPE_CONNECT:
		{
			char hostname[256]{};
			enet_address_get_host_ip(&event.peer->address, hostname, sizeof(hostname));
			std::cout << "A newclient connected from " << hostname << ":" << event.peer->address.port << std::endl;
		}
		break;
		case ENET_EVENT_TYPE_RECEIVE:
		{
			server_res_body_t server_res_body;
			memcpy(&server_res_body, event.packet->data, sizeof(server_res_body_t));
			handle_server_res_body(player_transform, network_info, server_res_body);
			enet_packet_destroy(event.packet);
		}
		break;

		case ENET_EVENT_TYPE_DISCONNECT:
		{
			printf("%s disconnected.\n", event.peer->data);
		}
		break;

		default:
		{
			printf("event type not handled");
		}
		break;
	}
}

void resync_ack_user_cmd(int player_handle, gameobject_snapshot_t& go_snapshot) {
	unsigned int user_cmd_response_id = go_snapshot.input_cmd_response;
	user_cmds_fifo_t::peek_state_t user_cmd_peek = user_cmds_fifo.peek_read();
	if (!user_cmd_peek.valid) return;
	if (user_cmd_response_id < user_cmd_peek.val->input_cmd_id) return;
	while (user_cmd_peek.valid && user_cmd_peek.val->input_cmd_id != user_cmd_response_id) {
		user_cmds_fifo.dequeue();
		user_cmd_peek = user_cmds_fifo.peek_read();
	}
	if (!user_cmd_peek.valid) return;
	user_cmds_fifo_t::dequeue_state_t dequeud = user_cmds_fifo.dequeue();
	user_cmd_t& orig_user_cmd = dequeud.val;
	transform_t* player_transform = get_transform(player_handle);
	
	// set player to correct position
	player_transform->position.x = go_snapshot.gameobject.x;
	player_transform->position.y = go_snapshot.gameobject.y;

	// re-simulate registered user inputs ater this one has been confirmed

	// TODO: eventually limit when you send inputs to only something was acc pressed so that at some pt, if player is not moving,
	// game can sync in what the player position is supposed to be b/c of last_invalid_snapshot on gameobject_snapshot_t
	// this can help sync in player completely while it is idle and the current snapshot id > last_invalid_snapshot but no inputs needs to be processed/predicted upon
	// via interpolation (this could acc save a lot of processing power)
	// (can go away from n^2 algorithm to something less on the average)
	for (int i = 0; i < user_cmds_fifo.get_size(); i++) {
		user_cmds_fifo_t::peek_state_t peek = user_cmds_fifo.get_at_idx(i);
		if (peek.valid) {
			user_cmd_t& user_cmd = *peek.val;
			user_input_t& user_input = user_cmd.user_input;
			input_state_t input_state;
			input_state.key_state.key_down['w'] = user_input.w_pressed;
			input_state.key_state.key_down['a'] = user_input.a_pressed;
			input_state.key_state.key_down['s'] = user_input.s_pressed;
			input_state.key_state.key_down['d'] = user_input.d_pressed;
			update_player_position(input_state, player_handle);
		}
	}
}

void handle_server_res_body(int player_transform_handle, basic_network_info_t& network_info, server_res_body_t& server_res) {
	if (server_res.res_type == RES_TYPE::JOIN_CODE)
	{
		std::cout << server_res.res_data.join_code << std::endl;
	}
	else if (server_res.res_type == RES_TYPE::RES_MSG)
	{
		std::cout << server_res.res_data.msg << std::endl;
	}
	else if (server_res.res_type == RES_TYPE::SNAPSHOT)
	{
		static unsigned int last_enqueued_snapshot_id = 0;
		snapshot_t& snapshot = server_res.res_data.snapshot_data;
		snapshots_fifo_t& snapshot_fifo = network_info.snapshots_fifo;
		
		snapshots_fifo_t::peek_state_t peek_state = snapshot_fifo.peek_read();
		// snapshot receiving out of order logic
		if (peek_state.valid && snapshot.snapshot_id < peek_state.val->snapshot_id) {
			std::cout << "cannot render this because it is too far back in interpolation time on client" << std::endl;
			return;
		}
		snapshot_fifo.enqueue(snapshot);
	} else if (server_res.res_type == RES_TYPE::GAMEOBJECT_SNAPSHOT) {
		// a response to our user cmd has been received, need to handle this accordingly
		gameobject_snapshot_t& go_snapshot = server_res.res_data.gameobject_snapshot;
		resync_ack_user_cmd(player_transform_handle, go_snapshot);
	}
}
#endif
