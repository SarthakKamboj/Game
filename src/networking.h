#pragma once

#include "enet/enet.h"
#include "shared/networking.h"

#define SERVER_IP "172.25.99.191"
#define SERVER_PORT 52985
// #define MAX_USERNAME_LEN 64

// enum REQ_TYPE {
//     REQ_NONE = 0,
//     CREATE_ROOM, 
//     JOIN_ROOM,
//     USER_CMD,
// };

// struct server_req_body_t {
//     union {
//         char username[MAX_USERNAME_LEN+1];
//         struct {
//             int room_join_code;
//             char username[MAX_USERNAME_LEN+1];
//         } join_room_info;
//     } req_data{};
//     REQ_TYPE req_type = REQ_TYPE::REQ_NONE;
// };

// enum RES_TYPE {
//     RES_NONE = 0,
//     JOIN_CODE,
//     RES_MSG
// };

// struct server_res_body_t {
//     union {
//         char msg[65]{};
//         int join_code;
//     } ack_data{};
//     RES_TYPE ack_type = RES_TYPE::RES_NONE;
// };

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