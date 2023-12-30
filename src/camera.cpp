#include "camera.h"
#include "glm/gtc/type_ptr.hpp"
#include "utils/time.h"
#include "constants.h"

#include "app.h"

extern application_t app;

camera_t::camera_t() {
	pos = glm::vec3(0, -10, 0);
	rotation = 0;
}

glm::mat4 camera_t::get_view_matrix() {
	glm::mat4 view_mat(1.0f);
	view_mat = glm::translate(view_mat, -pos);
	view_mat = glm::rotate(view_mat, glm::radians(-rotation), glm::vec3(0.f, 0.f, 1.0f));
	return view_mat;
}

void camera_t::update(input::user_input_t& user_input, main_character_t& main_char) {
	const float speed = 0.5f;
	float delta = static_cast<float>(platformer::time_t::delta_time * speed);
	transform_t* char_transform = get_transform(main_char.transform_handle);
	assert(char_transform);
	float normalized_player_pos_x = char_transform->position.x - pos.x;
	const float left_boundary = app.window_width * 1.f / 4.f;
	const float right_boundary = app.window_width * 3.f / 4.f;
	if (normalized_player_pos_x > right_boundary) {
		float amount_over = normalized_player_pos_x - right_boundary;
		pos.x += amount_over;
	}
	else if (normalized_player_pos_x < left_boundary) {
		float amount_under = left_boundary - normalized_player_pos_x;
		pos.x -= amount_under;
	}
}
