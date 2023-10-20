#pragma once

#include "glm/glm.hpp"
#include "constants.h" 
#include "input/input.h"

struct camera_t {
	glm::vec3 pos;
	float rotation;

	camera_t();
	glm::mat4 get_view_matrix();
	void update(input::user_input_t& user_input);
};
