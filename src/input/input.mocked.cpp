#include "input.h"

namespace input {
	void process_input(user_input_t& user_input) {}
	void handle_user_cmd_networking(user_input_net_state_t& state, user_input_t& user_input) {}
	void send_user_cmd(user_input_t& user_input) {}
	user_cmd_t clear_user_cmds_upto_id(unsigned int id) { return user_cmd_t();  }
	std::vector<user_cmd_t> get_cmds_after_id(unsigned int id) { return std::vector<user_cmd_t>(); }
}