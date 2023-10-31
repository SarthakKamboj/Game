#include "gos.h"
#include "constants.h" 
#include <iostream>
#include "utils/time.h"
#include <vector>

/**
 * @brief Store all the goombas in the world and all the turning points
*/
std::vector<goomba_t> goombas;
std::vector<goomba_turn_pt_t> goomba_turn_pts;

main_character_t create_main_character(const glm::vec3& pos, const glm::vec3& scale, float rot, glm::vec3& color, const glm::vec2& dims) {
	main_character_t mc;
	mc.transform_handle = create_transform(pos, scale, rot);
	mc.rec_render_handle = create_quad_render(mc.transform_handle, color, dims.x, dims.y, false, 0, -1);
	mc.rigidbody_handle = create_rigidbody(mc.transform_handle, true, dims.x, dims.y, false, PHYSICS_RB_TYPE::PLAYER);
	return mc;
}

void main_character_t::update(input::user_input_t& user_input) {

	bool prev_dead = dead;
	std::vector<general_collision_info_t>& col_infos = get_general_cols_for_non_kin_type(PHYSICS_RB_TYPE::PLAYER);
	for (general_collision_info_t& col_info : col_infos) {
		if (col_info.kin_type == PHYSICS_RB_TYPE::GROUND && col_info.rel_dir == PHYSICS_RELATIVE_DIR::BOTTOM) {
			grounded = true;
			num_jumps_since_grounded = 0;
		}
		if (col_info.kin_type == PHYSICS_RB_TYPE::GOOMBA) {
			if (col_info.rel_dir == PHYSICS_RELATIVE_DIR::BOTTOM) {
				delete_goomba_by_kin_handle(col_info.kin_handle);
				// goombas.clear();
				// rigidbody_t* kin_rb = get_rigidbody(col_info.kin_handle);
				// assert(kin_rb);
			}
			else {
				dead = true;
			}
		}
	}

    // get rigidbody and make sure its valid
    rigidbody_t* rb_ptr = get_rigidbody(rigidbody_handle);
    assert(rb_ptr != NULL);
	rigidbody_t& rb = *rb_ptr;

	if (!prev_dead && dead) {
		// static float time_elapsed = 0;
		rb.vel.y = WINDOW_HEIGHT / 2.f;
		rb.detect_col = false;
		return;
	}

	if (dead) return;

    // get velocity
	const float vel = WINDOW_WIDTH / 4.f;

    // jump
    bool jump_btn_pressed = user_input.w_pressed;
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

const glm::vec3 ground_block_t::BLOCK_COLOR = glm::vec3(1.f, 0.29f, 0.f);
int ground_block_t::tex_handle = -1;

ground_block_t create_ground_block(const glm::vec3& pos, const glm::vec3& scale, float rot) {
	ground_block_t block;
	block.transform_handle = create_transform(pos, scale, rot);
	glm::vec3 color = ground_block_t::BLOCK_COLOR;
	// block.rec_render_handle = create_rectangle_render(block.transform_handle, color, ground_block_t::WIDTH, ground_block_t::HEIGHT, false, 1.f, ground_block_t::tex_handle);
	block.rec_render_handle = create_quad_render(block.transform_handle, color, ground_block_t::WIDTH, ground_block_t::HEIGHT, false, 0.f, ground_block_t::tex_handle);
	block.rigidbody_handle = create_rigidbody(block.transform_handle, false, ground_block_t::WIDTH, ground_block_t::HEIGHT, true, PHYSICS_RB_TYPE::GROUND);
	return block;
}

const glm::vec3 goomba_t::GOOMBA_COLOR = glm::vec3(0.588f, 0.294f, 0.f);
void create_goomba(const glm::vec3& pos) {
	goomba_t goomba;
	goomba.transform_handle = create_transform(pos, glm::vec3(1), 0.f);
	glm::vec3 color = goomba_t::GOOMBA_COLOR;
	goomba.rec_render_handle = create_quad_render(goomba.transform_handle, color, goomba_t::WIDTH, goomba_t::HEIGHT, false, 0.f, -1);
	goomba.rigidbody_handle = create_rigidbody(goomba.transform_handle, false, goomba_t::WIDTH, goomba_t::HEIGHT, true, PHYSICS_RB_TYPE::GOOMBA);
	goombas.push_back(goomba);
}

void update_goomba(goomba_t& goomba) {
	transform_t* transform_ptr = get_transform(goomba.transform_handle);
	assert(transform_ptr);

	for (goomba_turn_pt_t& turn_pt : goomba_turn_pts) {
		glm::vec2 diff_vec = glm::vec2(turn_pt.x, turn_pt.y) - glm::vec2(transform_ptr->position.x, transform_ptr->position.y);
		if (glm::dot(diff_vec, diff_vec) <= 1.f) {
			goomba.move_speed *= -1.f;
		}
	}

	transform_ptr->position.x += goomba.move_speed * platformer::time_t::delta_time;
}

void delete_goomba_by_kin_handle(int kin_handle) {
	int i_to_remove = -1;
	for (goomba_t& goomba : goombas) {
		i_to_remove++;
		if (goomba.rigidbody_handle == kin_handle) {
			delete_kin_rigidbody(kin_handle);
			delete_quad_render(goomba.rec_render_handle);
			delete_transform(goomba.transform_handle);
			goombas.erase(goombas.begin() + i_to_remove);
			return;
		}
	}
}

void add_goomba_turn_point(glm::vec3 pos) {
	goomba_turn_pts.push_back(pos);
}

void gos_update() {
	for (goomba_t& goomba : goombas) {
		update_goomba(goomba);
	}
}


