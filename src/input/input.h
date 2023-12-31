#pragma once

struct application_t;

namespace input {

    /// <summary>
    /// Struct to store user input data
    /// </summary>
    struct user_input_t {

        bool some_key_pressed = false;

		// true just on the initial click, but false even if button is down
        bool w_pressed = false;
        bool a_pressed = false;
        bool s_pressed = false;
        bool d_pressed = false;
		bool p_pressed = false;
		bool l_pressed = false;
        bool space_pressed = false;
        bool enter_pressed = false;
        
        bool controller_a_pressed = false;
        bool controller_y_pressed = false;
        bool controller_x_pressed = false;
        bool controller_b_pressed = false;

		// true while continuously button is down
        bool w_down = false;
        bool a_down = false;
        bool s_down = false;
        bool d_down = false;
		bool p_down = false;
		bool l_down = false;
        bool enter_down = false;

        bool controller_a_down = false;
        bool controller_y_down = false;
        bool controller_x_down = false;
        bool controller_b_down = false;

        bool right_clicked = false;
        bool left_clicked = false;

        bool quit = false;

        int x_pos;
        int y_pos;

        float controller_x_axis = 0;
        float controller_y_axis = 0;
    };

    void init_controller(application_t& app);

	/// <summary>
	/// process the input for the frame
	/// </summary>
	/// <param name="user_input"></param>
	void process_input(application_t& app, user_input_t& user_input);
}
