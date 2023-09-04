#include "networking.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <math.h>
#include "utils/time.h"
#include "fifo.h"
#include "shared/networking.h"
#include "constants.h"

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

void handle_basic_frame_networking(basic_network_info_t& frame_basic_network_info) {
	assert(frame_basic_network_info.client != NULL);
	assert(frame_basic_network_info.server != NULL);
	assert(frame_basic_network_info.input_state != NULL);

	client_t& client = *frame_basic_network_info.client;
	server_t& server = *frame_basic_network_info.server;
	input_state_t& input_state = *frame_basic_network_info.input_state;
	handle_user_cmd_networking(frame_basic_network_info);
	network_event_t server_event;
	while (poll_server_event(client, server_event)) {
		handle_server_event(frame_basic_network_info, server_event);
	}
}

bool poll_server_event(client_t& client, network_event_t& network_event) {
	network_event.event_valid = enet_host_service(client.enet_host, &network_event.enet_event, 0) > 0;
	return network_event.event_valid;
}

void handle_user_cmd_networking(basic_network_info_t& basic_info) {
	basic_info.time_since_last_user_cmd_send += platformer::time_t::delta_time;
	const int SEND_INTERVAL = 1.0f/60.f;
	if (basic_info.time_since_last_user_cmd_send >= SEND_INTERVAL) {
		basic_info.time_since_last_user_cmd_send -= SEND_INTERVAL;
		send_user_cmd(*basic_info.client, *basic_info.server, basic_info.input_state->key_state);
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

void send_user_cmd(client_t &client, server_t &server, key_state_t &key_state)
{
	server_req_body_t server_body_req;
	server_body_req.req_type = REQ_TYPE::USER_CMD;

	// static unsigned int running_time = 0;

	auto &user_cmd = server_body_req.req_data.user_cmd_info;
	// user_cmd.game_time = running_time;
	user_cmd.game_time = platformer::get_time_since_start_in_sec();
	// const float SEND_INTERVAL = 1.0f/60.f;
	// user_cmd.delta_time = SEND_INTERVAL;
	user_cmd.w_pressed = key_state.key_down['w'];
	user_cmd.a_pressed = key_state.key_down['a'];
	user_cmd.s_pressed = key_state.key_down['s'];
	user_cmd.d_pressed = key_state.key_down['d'];

	// running_time++;

	ENetPacket *packet = enet_packet_create((void *)&server_body_req, sizeof(server_body_req), 0);
	enet_peer_send(server.enet_peer, 0, packet);
}

void handle_server_event(basic_network_info_t& network_info, network_event_t& network_event)
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
			handle_server_res_body(network_info, server_res_body);
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

void handle_server_res_body(basic_network_info_t& network_info, server_res_body_t& server_res) {
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
		snapshot_data_t& snapshot = server_res.res_data.snapshot_data;
		snapshots_fifo_t& snapshot_fifo = network_info.snapshots_fifo;
		
		snapshots_fifo_t::peek_state_t peek_state = snapshot_fifo.peek_read();
		// snapshot receiving out of order logic
		if (peek_state.valid && snapshot.snapshot_id < peek_state.val->snapshot_id) {
			std::cout << "cannot render this because it is too far back in interpolation time on client" << std::endl;
			return;
		}
		snapshot_fifo.enqueue(snapshot);
	}
}