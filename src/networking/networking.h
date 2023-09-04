#pragma once

#include "enet/enet.h"
#include "shared/networking.h"
#include "input/input.h"
#include "constants.h"

#define SERVER_IP "172.25.99.191"
#define SERVER_PORT 52985

struct client_t {
    ENetHost* enet_host = NULL;
    char username[65]{};
};

struct server_t {
    ENetPeer* enet_peer = NULL;
};

struct network_event_t {
    ENetEvent enet_event;
    bool event_valid = false;
};

struct basic_network_info_t {
    snapshots_fifo_t snapshots_fifo;
    client_t* client = NULL;
    server_t* server = NULL;
    input_state_t* input_state = NULL;
	float time_since_last_user_cmd_send = 0.f;
};

int init_networking(basic_network_info_t& basic_networking_info, client_t* client, server_t* server, input_state_t* input_state);
void cleanup_networking(basic_network_info_t& basic_networking_info);
client_t create_client(const char* username);
server_t find_game_server(client_t& client);
void send_create_room_req(client_t& client, server_t& server);
void send_join_room_req(client_t& client, server_t& server);
void handle_basic_frame_networking(basic_network_info_t& frame_basic_network_info);
void handle_user_cmd_networking(basic_network_info_t& basic_info);
void send_user_cmd(client_t& client, server_t& server, key_state_t& key_state);
bool poll_server_event(client_t& client, network_event_t& event);
void handle_server_event(basic_network_info_t& network_info, network_event_t& network_event);
void handle_server_res_body(basic_network_info_t& network_info, server_res_body_t& server_res);