#include "networking.h"
#include <iostream>
#include <cassert>
#include "utils/time.h"
#include "constants.h"

static input_state_t* input_state_ptr = NULL;

int init_networking(basic_network_info_t& basic_networking_info, client_t* client, server_t* server, input_state_t* input_state) {
	basic_networking_info.client = client;	
	basic_networking_info.server = server;	
	basic_networking_info.input_state = input_state;	
	basic_networking_info.time_since_last_user_cmd_send = 0.f;

    input_state_ptr = input_state;
	return 0;
}

void cleanup_networking(basic_network_info_t& basic_networking_info) {}

client_t create_client(const char *username)
{
    client_t client;
    client.enet_host = NULL;
    memcpy(client.username, username, strlen(username));
    return client;
}

server_t find_game_server(client_t &client)
{
    server_t server;
    server.enet_peer = NULL;
	return server;
}

void send_create_room_req(client_t &client, server_t &server) {}
void send_join_room_req(client_t &client, server_t &server) {}

void handle_basic_frame_networking(basic_network_info_t &frame_basic_network_info)
{
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

void handle_user_cmd_networking(basic_network_info_t& basic_info) { 
    basic_info.time_since_last_user_cmd_send += platformer::time_t::delta_time;
}

void send_user_cmd(client_t &client, server_t &server, key_state_t &key_state){}

float calc_new_time_between_snapshots(const int NUM_SNAPSHOTS_PER_SEC) {
	// 0 to 1
	float offset = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	// -0.5 to 0.5
	offset -= 0.5f;
	// -0.5 * (time between snaps) to 0.5 * (time between snaps)
	offset *= (1.f / NUM_SNAPSHOTS_PER_SEC);
	return (1.f / NUM_SNAPSHOTS_PER_SEC) + offset;
}

bool poll_server_event(client_t &client, network_event_t &event) {
    const int NUM_SNAPSHOTS_PER_SEC = 20;

    static float TIME_BETWEEN_SNAPSHOTS = calc_new_time_between_snapshots(NUM_SNAPSHOTS_PER_SEC);
    static float time_since_last_receive = 0.f;

    time_since_last_receive += platformer::time_t::delta_time;
    if (time_since_last_receive < TIME_BETWEEN_SNAPSHOTS) {
        event.event_valid = false;
        return false;
    }

    static int snapshot_id = 0;
    static float game_time_of_first_snapshot = platformer::time_t::cur_independent_time;

    float drop_factor = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

    if (drop_factor < 0.0125f)
    {
        snapshot_id++;
        std::cout << "dropped snapshot " << snapshot_id << std::endl;
        time_since_last_receive = 0.f;
        event.event_valid = false;
        return false;
    }

    time_since_last_receive -= TIME_BETWEEN_SNAPSHOTS;

    snapshot_data_t snapshot{};
    snapshot.snapshot_id = snapshot_id++;
    snapshot.game_time = platformer::time_t::cur_independent_time - game_time_of_first_snapshot;

    static float x = 0.f;
    static float y = 0.f;
    float speed = 10.0f;

    key_state_t& key_state = input_state_ptr->key_state;
    if (key_state.key_being_pressed['d'])
    {
        x += speed;
    }
    else if (key_state.key_being_pressed['a'])
    {
        x -= speed;
    }

    if (key_state.key_being_pressed['w'])
    {
        y += speed;
    }
    else if (key_state.key_being_pressed['s'])
    {
        y -= speed;
    }

    snapshot.gameobject_snapshots[0].x = x;
    snapshot.gameobject_snapshots[0].y = y;

    event.enet_event.type = ENET_EVENT_TYPE_RECEIVE;
    server_res_body_t server_res_body;
    server_res_body.res_type = RES_TYPE::SNAPSHOT;
    server_res_body.res_data.snapshot_data = snapshot;
 
    event.enet_event.packet = (ENetPacket*)malloc(sizeof(ENetPacket));
	assert(event.enet_event.packet != NULL);
	event.enet_event.packet->data = (enet_uint8*)malloc(sizeof(server_res_body_t));
	assert(event.enet_event.packet->data != NULL);
    memcpy(event.enet_event.packet->data, &server_res_body, sizeof(server_res_body_t));

    event.event_valid = true;
    TIME_BETWEEN_SNAPSHOTS = calc_new_time_between_snapshots(NUM_SNAPSHOTS_PER_SEC);

    return true;
}

void handle_server_event(basic_network_info_t& network_info, network_event_t &network_event) {
    if (!network_event.event_valid) return;
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
            free(event.packet);
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

void handle_server_res_body(basic_network_info_t& network_info, server_res_body_t &server_res) {
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