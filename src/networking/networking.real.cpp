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