#pragma once

namespace input {

    /// <summary>
    /// Struct to store user input data
    /// </summary>
    struct user_input_t {
		// true just on the initial click, but false even if button is down
        bool w_pressed = false;
        bool a_pressed = false;
        bool s_pressed = false;
        bool d_pressed = false;
		bool p_pressed = false;
        bool space_pressed = false;

		// true while continuously button is down
        bool w_down = false;
        bool a_down = false;
        bool s_down = false;
        bool d_down = false;
		bool p_down = false;

        bool right_clicked = false;
        bool left_clicked = false;

        bool quit = false;

        int x_pos;
        int y_pos;
    };

	/// <summary>
	/// process the input for the frame
	/// </summary>
	/// <param name="user_input"></param>
	void process_input(user_input_t& user_input);
}
