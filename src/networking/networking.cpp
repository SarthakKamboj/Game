#include "networking.h"
#include "update.h"
#include <cassert>
#include <vector>

std::vector<int> interpolated_obj_transforms;

namespace networking {

    void add_interpolated_obj(int transform_handle) {
		interpolated_obj_transforms.push_back(transform_handle);
	}

    void handle_network_event(network_event_t& network_event) {
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
				server_cmd_t* server_cmd_ptr = reinterpret_cast<server_cmd_t*>(network_event.enet_event.packet->data);
				assert(server_cmd_ptr != NULL);
				server_cmd_t& server_cmd = *server_cmd_ptr;
				switch (server_cmd.res_type) {
					case SERVER_CMD_TYPE::SNAPSHOT:
					case SERVER_CMD_TYPE::USER_CMD_ACK: {
						::world::receive_snapshot(server_cmd);
					}
						break;

					default:
						break;
				}
				destroy_network_event(network_event);
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

    void handle_network() {
		network_event_t event;
		while (poll_network_event(event)) {
			handle_network_event(event);
		}
	}
}