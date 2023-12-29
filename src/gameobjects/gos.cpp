#include "gos.h"
#include <iostream>
#include "utils/time.h"
#include <vector>
#include <unordered_set>
#include "app.h"
#include "animation/animation.h"
#include "renderer/opengl/resources.h"
#include "camera.h"
#include "audio/audio.h"
#include "utils/io.h"

#include "constants.h" 

struct pair_hash {
    inline std::size_t operator()(const std::pair<int,int> & v) const {
        return v.first*31+v.second;
    }
};

/**
 * @brief Store all the goombas in the world and all the turning points
*/
std::vector<goomba_t> goombas;
std::vector<pipe_t> pipes;
std::vector<goomba_turn_pt_t> goomba_turn_pts;
std::vector<brick_t> bricks;
std::vector<coin_t> coins;
std::vector<ice_power_up_t> ice_power_ups;
std::vector<ground_block_t> ground_blocks;
final_flag_t final_flag;
static std::unordered_set<std::pair<int, int>, pair_hash> created_positions;

void unload_level(application_t& app) {
	for (goomba_t& goomba : goombas) delete_goomba_by_kin_handle(goomba.rigidbody_handle);
	goombas.clear();
	goomba_turn_pts.clear();
	for (coin_t& coin : coins) delete_coin(coin);
	coins.clear();
	for (brick_t& brick : bricks) delete_brick(brick);
	bricks.clear();
	for (ice_power_up_t& ipu : ice_power_ups) delete_ice_powerup_by_kin_handle(ipu.rigidbody_handle);
	ice_power_ups.clear();
	delete_final_flag();
	memset(&final_flag, 0, sizeof(final_flag));

	for (int i = 0; i < ground_blocks.size(); i++) {
		delete_quad_render(ground_blocks[i].rec_render_handle);
		delete_transform(ground_blocks[i].transform_handle);
		delete_rigidbody(ground_blocks[i].rigidbody_handle);
	}
	ground_blocks.clear();
	created_positions.clear();

	for (int i = 0; i < pipes.size(); i++) {
		delete_quad_render(pipes[i].rec_render_handle);
		delete_transform(pipes[i].transform_handle);
		delete_rigidbody(pipes[i].rigidbody_handle);
	}
	pipes.clear();

	delete_mc(app.main_character);
}

int main_character_t::mc_statemachine_handle = -1;

const time_count_t main_character_t::DASH_TIME = 0.1;

void init_mc_data() {
	char resource_path[256]{};
	io::get_resources_folder_path(resource_path);

	char character_art_path[256]{};
	sprintf(character_art_path, "%s\\%s\\character", resource_path, ART_FOLDER);
	main_character_t::mc_statemachine_handle = create_state_machine(character_art_path, "mc", "character_running");
}

main_character_t create_main_character(const glm::vec3& pos, const glm::vec3& scale, float rot, glm::vec3& color) {
	main_character_t mc;
	mc.transform_handle = create_transform(pos, scale, rot);
	mc.rec_render_handle = create_quad_render(mc.transform_handle, color, GAME_GRID_SIZE, GAME_GRID_SIZE * 1.5, false, 1, -1);
	mc.rigidbody_handle = create_rigidbody(mc.transform_handle, true, GAME_GRID_SIZE * 0.8, GAME_GRID_SIZE * 1.6, false, PHYSICS_RB_TYPE::PLAYER, true, false);
	return mc;
}

void delete_mc(main_character_t& mc) {
	delete_transform(mc.transform_handle);
	delete_quad_render(mc.rec_render_handle);
	delete_rigidbody(mc.rigidbody_handle);
}

extern bool level_finished;
void main_character_t::update(application_t& app, input::user_input_t& user_input) {

	static bool waiting_for_finish_audio = false;
	if (waiting_for_finish_audio) {
		if (sound_finished_playing("level_finish")) {
			waiting_for_finish_audio = false;
			scene_manager_load_level(app.scene_manager, app.scene_manager.cur_level + 1);
		}
		return;
	}

	set_quad_texture(rec_render_handle, get_tex_handle_for_statemachine(mc_statemachine_handle));	

	bool prev_dead = dead;
	std::vector<general_collision_info_t>& col_infos = get_general_cols_for_non_kin_type(PHYSICS_RB_TYPE::PLAYER);

    // get rigidbody and make sure its valid
    rigidbody_t* rb_ptr = get_rigidbody(rigidbody_handle);
    game_assert(rb_ptr != NULL);
	rigidbody_t& rb = *rb_ptr;

	for (general_collision_info_t& col_info : col_infos) {

		if (col_info.kin_type == PHYSICS_RB_TYPE::FINAL_FLAG) {
			pause_bck_sound();
			play_sound("level_finish", true);
			waiting_for_finish_audio = true;
			continue;
		}

		bool hit_groundable_object = col_info.kin_type == PHYSICS_RB_TYPE::GROUND || col_info.kin_type == PHYSICS_RB_TYPE::BRICK;
		bool hit_obj_from_above = col_info.rel_dir == PHYSICS_RELATIVE_DIR::BOTTOM;
		if (hit_groundable_object && hit_obj_from_above) {
			grounded = true;
			num_jumps_since_grounded = 0;
		}
		else if (col_info.kin_type == PHYSICS_RB_TYPE::GOOMBA) {
			if (col_info.rel_dir == PHYSICS_RELATIVE_DIR::BOTTOM) {
    			play_sound("stomp");
				delete_goomba_by_kin_handle(col_info.kin_handle);
				rb.vel.y = WINDOW_HEIGHT / 3.f;
			}
			else {
				dead = true;
			}
		}
		else if (col_info.kin_type == PHYSICS_RB_TYPE::ICE_POWERUP) {
			color = glm::vec3(0.2f, 0.2f, 1.0f);
			delete_ice_powerup_by_kin_handle(col_info.kin_handle);
		}
	}

	if (!prev_dead && dead) {
		// static float time_elapsed = 0;
		rb.vel.y = WINDOW_HEIGHT / 2.f;
		rb.detect_col = false;
		return;
	}

	if (dead) {
		transform_t* t = get_transform(rb.transform_handle);
		if (t->position.y <= -750.f) {
			scene_manager_load_level(app.scene_manager, app.scene_manager.cur_level);
		}
		return;
	}

    // get velocity
	const float vel = WINDOW_WIDTH / 4.f;

	if (dashing_left) {
		dashing_left = (platformer::time_t::cur_time - dash_start_time) < DASH_TIME;
		if (!dashing_left) {
			rb.use_gravity = true;
		}
		rb.vel.x = -vel * 3.f;
		return;
	}

	if (dashing_right) {
		dashing_right = (platformer::time_t::cur_time - dash_start_time) < DASH_TIME;
		if (!dashing_right) {
			rb.use_gravity = true;
		}
		rb.vel.x = vel * 3.f;
		return;
	}

    // jump
    bool jump_btn_pressed = user_input.w_pressed;

	if (jump_btn_pressed) {
		if (grounded || num_jumps_since_grounded <= 1) {
			rb.vel.y = 2 * vel;
			grounded = false;
			num_jumps_since_grounded += 1;

    		play_sound("jump");
		}
	}

    bool move_left = user_input.a_down;
    bool move_right = user_input.d_down;

	bool dash_pressed = user_input.l_pressed;

	transform_t* t = get_transform(rb.transform_handle);
	if (move_left) {
		if (dash_pressed) {
			rb.vel.x = -vel * 3.f;
			dashing_left = true;
			dash_start_time = platformer::time_t::cur_time;
			rb.use_gravity = false;
			rb.vel.y = 0;
		} else {
			rb.vel.x = -vel;
		}
		set_state_machine_anim(mc_statemachine_handle, "character_running");
		t->y_deg = 180;
	}
	else if (move_right) {
		if (dash_pressed) {
			rb.vel.x = vel * 3.f;
			dashing_right = true;
			dash_start_time = platformer::time_t::cur_time;
			rb.use_gravity = false;
			rb.vel.y = 0;
		} else {
			rb.vel.x = vel;
		}
		set_state_machine_anim(mc_statemachine_handle, "character_running");
		t->y_deg = 0;
	}
	else {
		rb.vel.x = 0;
		set_state_machine_anim(mc_statemachine_handle, "character_idle");
	}

	if (rb.vel.y > 0) {
		set_state_machine_anim(mc_statemachine_handle, "character_jump_up");
	} else if (rb.vel.y < 0) {
		set_state_machine_anim(mc_statemachine_handle, "character_jump_down");
	}

}

const glm::vec3 ground_block_t::BLOCK_COLOR = glm::vec3(1.f, 0.29f, 0.f);
int ground_block_t::top_layer_tex_handle = -1;
int ground_block_t::bottom_layer_tex_handle = -1;
int ground_block_t::left_corner_tex_handle = -1;
int ground_block_t::left_tex_handle = -1;

void init_ground_block_data() {
	char resource_path[256]{};
	io::get_resources_folder_path(resource_path);

	char top_path[256]{};
	sprintf(top_path, "%s\\%s\\ground\\top.png", resource_path, ART_FOLDER);
	ground_block_t::top_layer_tex_handle = create_texture(top_path, 0);

	char bottom_path[256]{};
	sprintf(bottom_path, "%s\\%s\\ground\\bottom.png", resource_path, ART_FOLDER);
	ground_block_t::bottom_layer_tex_handle = create_texture(bottom_path, 0);

	char left_corner_path[256]{};
	sprintf(left_corner_path, "%s\\%s\\ground\\left_corner.png", resource_path, ART_FOLDER);
	ground_block_t::left_corner_tex_handle = create_texture(left_corner_path, 0);

	char left_path[256]{};
	sprintf(left_path, "%s\\%s\\ground\\left.png", resource_path, ART_FOLDER);
	ground_block_t::left_tex_handle = create_texture(left_path, 0);
}

void create_ground_block(const glm::vec3& pos, const glm::vec3& scale, float rot) {
	ground_block_t block;
	block.transform_handle = create_transform(pos, scale, rot);
	glm::vec3 color = ground_block_t::BLOCK_COLOR;
	bool block_above = created_positions.find(std::pair<int, int>(pos.x / GAME_GRID_SIZE, (pos.y / GAME_GRID_SIZE) + 1)) != created_positions.end();
	bool block_left = created_positions.find(std::pair<int, int>((pos.x / GAME_GRID_SIZE) - 1, pos.y / GAME_GRID_SIZE)) != created_positions.end();
	if (block_above) {
		if (!block_left) {
			block.rec_render_handle = create_quad_render(block.transform_handle, color, ground_block_t::WIDTH, ground_block_t::HEIGHT, false, 1.f, ground_block_t::left_tex_handle);
		} else {
			block.rec_render_handle = create_quad_render(block.transform_handle, color, ground_block_t::WIDTH, ground_block_t::HEIGHT, false, 1.f, ground_block_t::bottom_layer_tex_handle);
		}
	} else if (!block_above && !block_left) {
		block.rec_render_handle = create_quad_render(block.transform_handle, color, ground_block_t::WIDTH, ground_block_t::HEIGHT, false, 1.f, ground_block_t::left_corner_tex_handle);
	}
	else {
		block.rec_render_handle = create_quad_render(block.transform_handle, color, ground_block_t::WIDTH, ground_block_t::HEIGHT, false, 1.f, ground_block_t::top_layer_tex_handle);
	}
	created_positions.insert(std::pair<int, int>(pos.x / GAME_GRID_SIZE, pos.y / GAME_GRID_SIZE));

	block.rigidbody_handle = create_rigidbody(block.transform_handle, false, ground_block_t::WIDTH, ground_block_t::HEIGHT, true, PHYSICS_RB_TYPE::GROUND, true, false);
	ground_blocks.push_back(block);
	// return block;
}

const glm::vec3 goomba_t::GOOMBA_COLOR = glm::vec3(0.588f, 0.294f, 0.f);

void init_goomba_data() {}

void create_goomba(const glm::vec3& pos) {
	goomba_t goomba;
	static int running_cnt = 0;
	goomba.handle = running_cnt;
	running_cnt++;
	goomba.transform_handle = create_transform(pos, glm::vec3(1), 0.f, 180.f);
	glm::vec3 color = goomba_t::GOOMBA_COLOR;
	goomba.rec_render_handle = create_quad_render(goomba.transform_handle, color, goomba_t::WIDTH, goomba_t::HEIGHT / 2.f, false, 1.f, -1);
	goomba.rigidbody_handle = create_rigidbody(goomba.transform_handle, false, goomba_t::WIDTH, goomba_t::HEIGHT, true, PHYSICS_RB_TYPE::GOOMBA, true, false);
	char sm_name[64]{};
	sprintf(sm_name, "goomba_%i", goomba.handle);

	char resource_path[256]{};
	io::get_resources_folder_path(resource_path);

	char enemy_folder_path[256]{};
	sprintf(enemy_folder_path, "%s\\%s\\enemy1", resource_path, ART_FOLDER);

	goomba.statemachine_handle = create_state_machine(enemy_folder_path, sm_name, "enemy1_idle");
	goombas.push_back(goomba);
}

void update_goomba(goomba_t& goomba) {

	set_quad_texture(goomba.rec_render_handle, get_tex_handle_for_statemachine(goomba.statemachine_handle));

	transform_t* transform_ptr = get_transform(goomba.transform_handle);
	game_assert(transform_ptr);

	for (goomba_turn_pt_t& turn_pt : goomba_turn_pts) {
		glm::vec2 diff_vec = glm::vec2(turn_pt.x, turn_pt.y) - glm::vec2(transform_ptr->position.x, transform_ptr->position.y);
		if (glm::dot(diff_vec, diff_vec) <= 1.f) {
			goomba.move_speed *= -1.f;
			if (goomba.move_speed > 0) {
				transform_ptr->y_deg = 0;
			} else {
				transform_ptr->y_deg = 180.f;
			}
		}
	}

	transform_ptr->position.x += goomba.move_speed * platformer::time_t::delta_time;
}

void delete_goomba_by_kin_handle(int kin_handle) {
	int i_to_remove = -1;
	for (goomba_t& goomba : goombas) {
		i_to_remove++;
		if (goomba.rigidbody_handle == kin_handle) {
			delete_rigidbody(kin_handle);
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

const glm::vec3 pipe_t::PIPE_COLOR = glm::vec3(0.1f, 0.95f, 0.3f);
void create_pipe(glm::vec3 bottom_pos) {
	pipe_t pipe;
	bottom_pos.y -= 20.f;
	bottom_pos.y += pipe_t::HEIGHT / 2.f;
	pipe.transform_handle = create_transform(bottom_pos, glm::vec3(1), 0.f);
	glm::vec3 color = pipe_t::PIPE_COLOR;
	pipe.rec_render_handle = create_quad_render(pipe.transform_handle, color, pipe_t::WIDTH, pipe_t::HEIGHT, false, 0.f, -1);
	pipe.rigidbody_handle = create_rigidbody(pipe.transform_handle, false, pipe_t::WIDTH, pipe_t::HEIGHT, true, PHYSICS_RB_TYPE::GROUND);
	pipes.push_back(pipe);
}

int brick_t::unbroken_tex_handle = -1;
int brick_t::broken_tex_handle = -1;

void init_brick_data() {
	char resource_path[256]{};
	io::get_resources_folder_path(resource_path);

	char unbroken_path[256]{};
	sprintf(unbroken_path, "%s\\%s\\brick\\unbroken.png", resource_path, ART_FOLDER);
	char broken_path[256]{};
	sprintf(broken_path, "%s\\%s\\brick\\broken.png", resource_path, ART_FOLDER);

	brick_t::unbroken_tex_handle = create_texture(unbroken_path, 0);
	brick_t::broken_tex_handle = create_texture(broken_path, 0);
}

const glm::vec3 brick_t::BRICK_COLOR = glm::vec3(149.f, 52.f, 28.f) / 255.f;
void create_brick(glm::vec3 pos) {
	static int i = 0;
	brick_t brick;
	brick.handle = i;
	i++;
	brick.transform_handle = create_transform(pos, glm::vec3(1), 0.f);
	glm::vec3 color = brick_t::BRICK_COLOR;
	brick.rec_render_handle = create_quad_render(brick.transform_handle, color, brick_t::WIDTH, brick_t::HEIGHT, false, 1.f, brick_t::unbroken_tex_handle);
	brick.rigidbody_handle = create_rigidbody(brick.transform_handle, false, brick_t::WIDTH, brick_t::HEIGHT, true, PHYSICS_RB_TYPE::BRICK, true, false);
	bricks.push_back(brick);
}

void update_brick(brick_t& brick) {
	std::vector<general_collision_info_t>& cols = get_general_cols_for_kin(brick.rigidbody_handle, PHYSICS_RB_TYPE::BRICK);
	for (general_collision_info_t& col : cols) {
		if (col.non_kin_type == PHYSICS_RB_TYPE::PLAYER && col.rel_dir == PHYSICS_RELATIVE_DIR::TOP) {
			if (!brick.created_powerup) {
				transform_t* t = get_transform(brick.transform_handle);
				game_assert(t);
				// create_coin(t->position);
				// create_ice_powerup(t->position);
				// delete_brick(brick);
				brick.created_powerup = true;
				set_quad_texture(brick.rec_render_handle, brick_t::broken_tex_handle);
			}
		}
	}
}

void delete_brick(brick_t& brick) {
	delete_quad_render(brick.rec_render_handle);
	delete_rigidbody(brick.rigidbody_handle);
	delete_transform(brick.transform_handle);
	for (int i = 0; i < bricks.size(); i++) {
		if (bricks[i].handle == brick.handle) {
			bricks.erase(bricks.begin() + i);
			return;
		}
	}
}

const glm::vec3 coin_t::COIN_COLOR = glm::vec3(255.f, 169.f, 8.f) / 255.f;
void create_coin(glm::vec3 pos) {
	static int i = 0;
	coin_t coin;
	coin.handle = i;
	i++;
	coin.transform_handle = create_transform(pos, glm::vec3(1), 0.f);
	coin.start_pos = pos;
	coin.creation_time = platformer::time_t::cur_time;
	glm::vec3 color = coin_t::COIN_COLOR;
	coin.rec_render_handle = create_quad_render(coin.transform_handle, color, coin_t::WIDTH, coin_t::HEIGHT, false, 0.f, -1);
	coins.push_back(coin);
}

const float coin_t::MOVE_VERT_ANIM = 40.f;
const float coin_t::ANIM_TIME = 0.15f;
void update_coin(coin_t& coin) {
	transform_t* t = get_transform(coin.transform_handle);
	game_assert(t);
	float time_elapsed = platformer::time_t::cur_time - coin.creation_time;
	t->position.y = coin.start_pos.y + (time_elapsed / coin_t::ANIM_TIME * coin_t::MOVE_VERT_ANIM);
	if (time_elapsed >= coin_t::ANIM_TIME) {
		delete_coin(coin);
	}
}

void delete_coin(coin_t& coin) {
	for (int i = 0; i < coins.size(); i++) {
		if (coins[i].handle == coin.handle) {
			delete_quad_render(coin.rec_render_handle);
			delete_transform(coin.transform_handle);
			coins.erase(coins.begin() + i);
		}
	}
}

const glm::vec3 ice_power_up_t::ICE_POWERUP_COLOR = glm::vec3(0, 0, 1);
void create_ice_powerup(glm::vec3 pos) {
	static int i = 0;
	ice_power_up_t ice;
	ice.handle = i;
	i++;
	ice.transform_handle = create_transform(pos, glm::vec3(1), 0.f);
	ice.start_y_pos = pos.y;
	glm::vec3 color = ice_power_up_t::ICE_POWERUP_COLOR;
	ice.rec_render_handle = create_quad_render(ice.transform_handle, color, ice_power_up_t::WIDTH, ice_power_up_t::HEIGHT, false, 0.f, -1);
	ice.rigidbody_handle = create_rigidbody(ice.transform_handle, false, ice_power_up_t::WIDTH, ice_power_up_t::HEIGHT, false, PHYSICS_RB_TYPE::ICE_POWERUP, false);
	ice.creation_time = platformer::time_t::cur_time;
	ice_power_ups.push_back(ice);
}

void update_ice_powerup(ice_power_up_t& power_up) {
	float time_since_created = platformer::time_t::cur_time - power_up.creation_time;
	transform_t* t = get_transform(power_up.transform_handle);
	game_assert(t);
	if (time_since_created < 1.f) {
		const float TOTAL_MOVE_Y = ((brick_t::HEIGHT / 2) + (ice_power_up_t::HEIGHT / 2)) * 1.2f;
		t->position.y = power_up.start_y_pos + (TOTAL_MOVE_Y * time_since_created);
		return;
	}

	rigidbody_t* rb = get_rigidbody(power_up.rigidbody_handle);
	game_assert(rb);
	rb->use_gravity = true;
	rb->detect_col = true;

	const float move_speed = 100.f;
	t->position.x += move_speed * platformer::time_t::delta_time;
}

// void delete_ice_powerup(ice_power_up_t& power_up) {
void delete_ice_powerup_by_kin_handle(int kin_handle) {
	for (int i = 0; i < ice_power_ups.size(); i++) {
		if (ice_power_ups[i].rigidbody_handle == kin_handle) {
			ice_power_up_t& power_up = ice_power_ups[i];
			delete_quad_render(power_up.rec_render_handle);
			delete_rigidbody(power_up.rigidbody_handle);
			delete_transform(power_up.transform_handle);
			ice_power_ups.erase(ice_power_ups.begin() + i);
		}
	}
}

int final_flag_t::tex_handle = -1;
glm::vec3 final_flag_t::FINAL_FLAG_COLOR = glm::vec3(1, 1, 0);

void init_final_flag_data() {
	char resource_path[256]{};
	io::get_resources_folder_path(resource_path);
	char final_texture_path[256]{};
	sprintf(final_texture_path, "%s\\%s\\final\\final.png", resource_path, ART_FOLDER);

	final_flag_t::tex_handle = create_texture(final_texture_path, 0);
}

void create_final_flag(glm::vec3 pos) {
	memset(&final_flag, 0, sizeof(final_flag));
	glm::vec3 center_origin = pos + glm::vec3(0, -20 + final_flag_t::HEIGHT/2, 0);
	glm::vec3 x = center_origin;
	final_flag.transform_handle = create_transform(center_origin, glm::vec3(1), 0.f);
	int render_transform = create_transform(pos + glm::vec3(-20 + (final_flag_t::RENDER_WIDTH/2), -20 + (final_flag_t::RENDER_HEIGHT/2), 0), glm::vec3(1), 0.f); 
	final_flag.rec_render_handle = create_quad_render(render_transform, final_flag_t::FINAL_FLAG_COLOR, final_flag_t::RENDER_WIDTH, final_flag_t::RENDER_HEIGHT, false, 1.f, final_flag_t::tex_handle);
	final_flag.rigidbody_handle = create_rigidbody(final_flag.transform_handle, false, final_flag_t::WIDTH, final_flag_t::HEIGHT, true, PHYSICS_RB_TYPE::FINAL_FLAG, true, false);
}

void delete_final_flag() {
	delete_transform(final_flag.transform_handle);
	delete_quad_render(final_flag.rec_render_handle);
	delete_rigidbody(final_flag.rigidbody_handle);
}

int parallax_bck::bck_texture = -1;
int parallax_bck::transform_handles[2] = {-1, -1};
int parallax_bck::rec_render_handles[2] = {-1, -1};

void init_parallax_bck_data() {
	int ground_height = -10;
	parallax_bck::transform_handles[0] = create_transform(glm::vec3(WINDOW_WIDTH/2, ground_height + (WINDOW_HEIGHT-ground_height)/2, 0), glm::vec3(1), 0.f, 0.f);
	parallax_bck::transform_handles[1] = create_transform(glm::vec3(WINDOW_WIDTH + WINDOW_WIDTH/2, ground_height + (WINDOW_HEIGHT-ground_height)/2, 0), glm::vec3(1), 0.f, 0.f);

	char resource_path[256]{};
	io::get_resources_folder_path(resource_path);
	char bck_texture_path[256]{};
	sprintf(bck_texture_path, "%s\\%s\\background\\background.png", resource_path, ART_FOLDER);
	parallax_bck::bck_texture = create_texture(bck_texture_path, 0);
	parallax_bck::rec_render_handles[0] = create_quad_render(parallax_bck::transform_handles[0], glm::vec3(1), WINDOW_WIDTH, WINDOW_HEIGHT-ground_height, false, 1.f, parallax_bck::bck_texture);
	parallax_bck::rec_render_handles[1] = create_quad_render(parallax_bck::transform_handles[1], glm::vec3(1), WINDOW_WIDTH, WINDOW_HEIGHT-ground_height, false, 1.f, parallax_bck::bck_texture);
}

void update_parallax_bcks(camera_t& camera) {
	int cam_x = camera.pos.x;
	int window_width_offset = cam_x / WINDOW_WIDTH;

	transform_t* even_bck = get_transform(parallax_bck::transform_handles[0]);
	if (window_width_offset % 2 == 0) {
		even_bck->position.x = (window_width_offset * WINDOW_WIDTH) + (WINDOW_WIDTH / 2);
	} else {
		even_bck->position.x = ((window_width_offset + 1) * WINDOW_WIDTH) + (WINDOW_WIDTH / 2);
	}

	transform_t* odd_bck = get_transform(parallax_bck::transform_handles[1]);
	if (window_width_offset % 2 == 1) {
		odd_bck->position.x = (window_width_offset * WINDOW_WIDTH) + (WINDOW_WIDTH / 2);
	} else {
		odd_bck->position.x = ((window_width_offset + 1) * WINDOW_WIDTH) + (WINDOW_WIDTH / 2);
	}
}

void gos_update() {
	for (goomba_t& goomba : goombas) {
		update_goomba(goomba);
	}

	for (brick_t& brick : bricks) {
		update_brick(brick);
	}

	for (coin_t& coin : coins) {
		update_coin(coin);
	}

	for (ice_power_up_t& power_up : ice_power_ups) {
		update_ice_powerup(power_up);
	}
}

