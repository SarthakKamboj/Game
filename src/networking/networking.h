#pragma once

#include "enet/enet.h"
#include "shared/networking/networking.h"
#include "constants.h"

#define SERVER_IP "172.25.99.191"
#define SERVER_PORT 52985

namespace networking {
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

    struct network_state_t {
        client_t client;
        server_t server;
    }; 

    int init_networking();
    void cleanup_networking();
    bool poll_network_event(network_event_t& network_event);
    void handle_network_event(network_event_t& network_event);
    void destroy_network_event(network_event_t& network_event);
    void handle_network();
    client_t create_client(const char* username);
    server_t find_game_server(client_t& client);
    void send_client_cmd(client_cmd_t& client_cmd, bool reliable);
    void add_interpolated_obj(int transform_handle);
}