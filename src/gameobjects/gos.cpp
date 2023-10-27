#include "gos.h"
#include "constants.h" 
#include <iostream>
#include "utils/time.h"

main_character_t create_main_character(const glm::vec3& pos, const glm::vec3& scale, float rot, glm::vec3& color, const glm::vec2& dims) {
	main_character_t mc;
	mc.transform_handle = create_transform(pos, scale, rot);
	mc.rec_render_handle = create_quad_render(mc.transform_handle, color, dims.x, dims.y, false, 0, -1);
	mc.rigidbody_handle = create_rigidbody(mc.transform_handle, true, dims.x, dims.y, false, PHYSICS_RB_TYPE::PLAYER);
	return mc;
}

void main_character_t::update(input::user_input_t& user_input) {

	std::vector<general_collision_info_t>& col_infos = get_general_cols_for_non_kin_type(PHYSICS_RB_TYPE::PLAYER);
	for (general_collision_info_t& col_info : col_infos) {
		if (col_info.kin_type == PHYSICS_RB_TYPE::GROUND && col_info.rel_dir == PHYSICS_RELATIVE_DIR::BOTTOM) {
			grounded = true;
			num_jumps_since_grounded = 0;
		}
	}

    // get velocity
	const float vel = WINDOW_WIDTH / 4.f;

    // get rigidbody and make sure its valid
    rigidbody_t* rb_ptr = get_rigidbody(rigidbody_handle);
    assert(rb_ptr != NULL);
	rigidbody_t& rb = *rb_ptr;

    // jump
    bool jump_btn_pressed = user_input.space_pressed;
    bool character_falling = rb.vel.y <= 0;
	if (jump_btn_pressed) {
		if (grounded || num_jumps_since_grounded <= 1) {
			rb.vel.y = 2 * vel;
			grounded = false;
			num_jumps_since_grounded += 1;
		}
	}

    bool left_move_pressed = user_input.a_down;
    bool right_move_pressed = user_input.d_down;

	if (left_move_pressed) {
		rb.vel.x = -vel;
	}
	else if (right_move_pressed) {
		rb.vel.x = vel;
	}
	else {
		rb.vel.x = 0;
	}

}

const glm::vec3 ground_block_t::BLOCK_COLOR = glm::vec3(0.58f, 0.29f, 0.f);
int ground_block_t::tex_handle = -1;

ground_block_t create_ground_block(const glm::vec3& pos, const glm::vec3& scale, float rot) {
	ground_block_t block;
	block.transform_handle = create_transform(pos, scale, rot);
	glm::vec3 color = ground_block_t::BLOCK_COLOR;
	// block.rec_render_handle = create_rectangle_render(block.transform_handle, color, ground_block_t::WIDTH, ground_block_t::HEIGHT, false, 1.f, ground_block_t::tex_handle);
	block.rec_render_handle = create_quad_render(block.transform_handle, color, ground_block_t::WIDTH, ground_block_t::HEIGHT, false, 1.f, ground_block_t::tex_handle);
	block.rigidbody_handle = create_rigidbody(block.transform_handle, false, ground_block_t::WIDTH, ground_block_t::HEIGHT, true, PHYSICS_RB_TYPE::GROUND);
	return block;
}
