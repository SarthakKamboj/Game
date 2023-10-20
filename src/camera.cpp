#include "camera.h"
#include "glm/gtc/type_ptr.hpp"
#include "utils/time.h"

camera_t::camera_t() {
	// pos = glm::vec3(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0);
	pos = glm::vec3(0, -10, 0);
	rotation = 0;
}

glm::mat4 camera_t::get_view_matrix() {
	glm::mat4 view_mat(1.0f);
	view_mat = glm::translate(view_mat, -pos);
	view_mat = glm::rotate(view_mat, glm::radians(-rotation), glm::vec3(0.f, 0.f, 1.0f));
	return view_mat;
}

void camera_t::update(input::user_input_t& user_input) {
	const float speed = 0.5f;
	float delta = static_cast<float>(platformer::time_t::delta_time * speed);
	if (user_input.d_down) {
		pos.x += speed;
	}
	else if (user_input.a_down) {
		pos.x -= speed;
	}
}
