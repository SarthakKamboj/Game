#include "networking.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <math.h>

client_t create_client(const char* username) {
    client_t client;
    ENetHost* enet_host = enet_host_create(NULL, 1, 2, 0, 0);
    assert(enet_host != NULL);
    client.enet_host = enet_host;
    memcpy(client.username, username, fmin(strlen(username)+1, MAX_USERNAME_LEN+1));
    return client;
}

void send_create_room_req(client_t& client, server_t& server) {
    server_req_body_t server_req;
    server_req.req_type = REQ_TYPE::CREATE_ROOM;
    memcpy(server_req.req_data.username, client.username, strlen(client.username)+1);

	ENetPacket *packet = enet_packet_create((void*)&server_req, sizeof(server_req), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(server.enet_peer, 0, packet);

	enet_host_flush(client.enet_host);
}

void send_join_room_req(client_t& client, server_t& server) {
    server_req_body_t server_req;
    server_req.req_type = REQ_TYPE::JOIN_ROOM;
    memcpy(server_req.req_data.join_room_info.username, client.username, strlen(client.username)+1);
    server_req.req_data.join_room_info.room_join_code = 10245;

	ENetPacket *packet = enet_packet_create((void*)&server_req, sizeof(server_req), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(server.enet_peer, 0, packet);

	enet_host_flush(client.enet_host);
}

server_t find_game_server(client_t& client) {
    server_t server;

    ENetAddress address;
	enet_address_set_host(&address, SERVER_IP);
	address.port = SERVER_PORT;

	ENetPeer* peer = enet_host_connect(client.enet_host, &address, 2, 0);
	assert(peer != NULL);

	ENetEvent conn_event;
	int num_conn_evnts = enet_host_service(client.enet_host, &conn_event, 5000);
	if (num_conn_evnts > 0) {
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
		} else if (conn_event.type == ENET_EVENT_TYPE_DISCONNECT)
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