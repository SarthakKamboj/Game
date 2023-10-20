#pragma once

#if 1

namespace input {

    /// <summary>
    /// Struct to store user input data
    /// </summary>
    struct user_input_t {
        bool w_pressed = false;
        bool a_pressed = false;
        bool s_pressed = false;
        bool d_pressed = false;
        bool space_pressed = false;

        bool w_down = false;
        bool a_down = false;
        bool s_down = false;
        bool d_down = false;
    };

	/// <summary>
	/// process the input for the frame
	/// </summary>
	/// <param name="user_input"></param>
	void process_input(user_input_t& user_input);
}

#else

#include "shared/input/input.h"
#include <vector>

namespace input {
	/// <summary>
	/// process the input for the frame
	/// </summary>
	/// <param name="user_input"></param>
	void process_input(user_input_t& user_input);
	void handle_user_cmd_networking(user_input_net_state_t& state, user_input_t& user_input);
	void send_user_cmd(user_input_t& user_input);
	user_cmd_t clear_user_cmds_upto_id(unsigned int id);
	std::vector<user_cmd_t> get_cmds_after_id(unsigned int id);
}
#endif