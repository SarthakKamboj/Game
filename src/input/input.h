#pragma once

#include "shared/input/input.h"
#include <vector>

namespace input {
	void process_input(user_input_t& user_input);
	void handle_user_cmd_networking(user_input_net_state_t& state, user_input_t& user_input);
	void send_user_cmd(user_input_t& user_input);
	user_cmd_t clear_user_cmds_upto_id(unsigned int id);
	std::vector<user_cmd_t> get_cmds_after_id(unsigned int id);
}