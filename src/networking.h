#pragma once

#include "enet/enet.h"
#include "shared/networking.h"
#include "input/input.h"

#define SERVER_IP "172.25.99.191"
#define SERVER_PORT 52985

struct client_t {
    ENetHost* enet_host = NULL;
    char username[65]{};
};

struct server_t {
    ENetPeer* enet_peer = NULL;
};

client_t create_client(const char* username);
server_t find_game_server(client_t& client);
void send_create_room_req(client_t& client, server_t& server);
void send_join_room_req(client_t& client, server_t& server);
void send_user_cmd(client_t& client, server_t& server, key_state_t& key_state);