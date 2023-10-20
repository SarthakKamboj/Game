#include "gos.h"
#include "constants.h" 
#include <iostream>
#include "utils/time.h"

main_character_t create_main_character(const glm::vec3& pos, const glm::vec3& scale, float rot, glm::vec3& color, const glm::vec2& dims) {
	main_character_t mc;
	mc.transform_handle = create_transform(pos, scale, rot);
	mc.rec_render_handle = create_quad_render(mc.transform_handle, color, dims.x, dims.y, false, 0, -1);
	mc.rigidbody_handle = create_rigidbody(mc.transform_handle, true, dims.x, dims.y, false);
	return mc;
}

void main_character_t::update(input::user_input_t& user_input) {
#if 1
    // get velocity
	const float vel = WINDOW_WIDTH / 4.f;

    // get rigidbody and make sure its valid
    rigidbody_t* rb_ptr = get_rigidbody(rigidbody_handle);
    assert(rb_ptr != NULL);
	rigidbody_t& rb = *rb_ptr;

    // jump
    bool jump_btn_pressed = user_input.w_down;
    bool character_falling = rb.vel.y <= 0;
	if (jump_btn_pressed && character_falling) {
		rb.vel.y = 2*vel;
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
#else
        glm::vec2 delta(0);
        if (input_state.d_down) {
            delta.x = 1;
        }
        else if (input_state.a_down) {
            delta.x = -1;
        }

        if (input_state.w_down) {
            delta.y = 1;
        }
        else if (input_state.s_down) {
            delta.y = -1;
        }

        if (delta.x != 0 || delta.y != 0) {
            const int speed = 200;
            delta = glm::normalize(delta) * static_cast<float>(platformer::time_t::delta_time * speed);
            // delta = glm::normalize(delta) * 2.f;
        }
        transform_t* player_transform = get_transform(transform_handle);
        assert(player_transform);
        player_transform->position += glm::vec3(delta, 0);
#endif
}

#if 0
void update_main_character(const main_character_t& mc, input::user_input_t& user_input) {
 
}
#endif

const glm::vec3 ground_block_t::BLOCK_COLOR = glm::vec3(0.58f, 0.29f, 0.f);
int ground_block_t::tex_handle = -1;

ground_block_t create_ground_block(const glm::vec3& pos, const glm::vec3& scale, float rot) {
	ground_block_t block;
	block.transform_handle = create_transform(pos, scale, rot);
	glm::vec3 color = ground_block_t::BLOCK_COLOR;
	// block.rec_render_handle = create_rectangle_render(block.transform_handle, color, ground_block_t::WIDTH, ground_block_t::HEIGHT, false, 1.f, ground_block_t::tex_handle);
	block.rec_render_handle = create_quad_render(block.transform_handle, color, ground_block_t::WIDTH, ground_block_t::HEIGHT, false, 1.f, ground_block_t::tex_handle);
	block.rigidbody_handle = create_rigidbody(block.transform_handle, false, ground_block_t::WIDTH, ground_block_t::HEIGHT, true);
	return block;
}
