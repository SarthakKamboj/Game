#pragma once

#include "constants.h"
#include "renderer/basic/shape_renders.h"
#include "transform/transform.h"
#include "input/input.h"
#include "physics/physics.h"
#include "glm/glm.hpp"
#include "utils/time.h"

/**
 * @brief Update all gameobjects
*/
void gos_update();

struct application_t;
struct main_character_t {
	// create transform first
	int transform_handle = -1;
	// character currently renders a rectangle, will later change to sprite
	int rec_render_handle = -1;
    // rigidbody for the character
	int rigidbody_handle = -1;
	glm::vec3 color;
	glm::vec2 dims;

	static int mc_statemachine_handle;

	bool grounded = false;
	int num_jumps_since_grounded = 0;
	bool dead = false;

	bool waiting_for_level_finish_audio = false;

	bool dashing_left = false;
	bool dashing_right = false;
	time_count_t dash_start_time = 0;
	static const time_count_t DASH_TIME;
	static const time_count_t DASH_WAIT_TIME;

	void update(application_t& app, input::user_input_t& user_input);
};

void init_mc_data();
main_character_t create_main_character(const glm::vec3& pos, const glm::vec3& scale, float rot, glm::vec3& color);
void delete_mc(main_character_t& mc);

struct ground_block_t {
	// create transform first
	int transform_handle = -1;
	// block currently renders a rectangle, will later change to sprite
	int rec_render_handle = -1;
	int rigidbody_handle = -1;
	glm::vec3 color;
	static const int WIDTH = GAME_GRID_SIZE;
	static const int HEIGHT = GAME_GRID_SIZE;
	static const glm::vec3 BLOCK_COLOR;

    static int top_layer_tex_handle;
    static int bottom_layer_tex_handle;
    static int left_corner_tex_handle;
    static int left_tex_handle;
};

void init_ground_block_data();
void create_ground_block(const glm::vec3& pos, const glm::vec3& scale, float rot);

typedef glm::vec3 goomba_turn_pt_t;

/**
 * @brief Represents a Goomba
*/
struct goomba_t {
	int handle = -1;
	int transform_handle = -1;
	int rec_render_handle = -1;
	int statemachine_handle = -1;
	int rigidbody_handle = -1;
	static const int HEIGHT = GAME_GRID_SIZE;	
	static const int WIDTH = goomba_t::HEIGHT * 1.1f;
	static const glm::vec3 GOOMBA_COLOR;
	float move_speed = -50.f;

	static const time_count_t TURN_WAIT_TIME;
	time_count_t last_turn_time = -goomba_t::TURN_WAIT_TIME;
};

void init_goomba_data();

/**
 * @brief Create a goomba at a particular position
 * @param pos 
*/
void create_goomba(const glm::vec3& pos);

/**
 * @brief Update a Goomba
 * @param goomba 
*/
void update_goomba(goomba_t& goomba);

/**
 * @brief Add a point in the world where goombas turn around to make sure
 * they don't move through ground and walls
 * @param pos Position of the turning point
*/
void add_goomba_turn_point(glm::vec3 pos);

/**
 * @brief Delete a goomba based on its rigidbody handle
 * @param kin_handle Rigidbody handle of the goomba
*/
void delete_goomba_by_kin_handle(int kin_handle);

/**
 * @brief Pipe
*/
struct pipe_t {
	// create transform first
	int transform_handle = -1;
	int rec_render_handle = -1;
	int rigidbody_handle = -1;
	glm::vec3 color;
	static const int WIDTH = GAME_GRID_SIZE;
	static const int HEIGHT = GAME_GRID_SIZE * 2;
	static const glm::vec3 PIPE_COLOR;
};

/**
 * @brief Create a pipe with the relative point the bottom of the pipe
 * @param bottom_pos The position of the very bottom of the pipe, not the center. Usually for other
 * gameobjects we specify the center but here we specify the bottom of the pipe since pipes are not squares
*/
void create_pipe(glm::vec3 bottom_pos);

struct coin_t {
	int handle = -1;
	int transform_handle = -1;
	int rec_render_handle = -1;
	// int rigidbody_handle = -1;
	glm::vec3 color;
	glm::vec3 start_pos;
	time_count_t creation_time;
	static const int WIDTH = 10;
	static const int HEIGHT = 10;
	static const glm::vec3 COIN_COLOR;
	static const float MOVE_VERT_ANIM;
	static const float ANIM_TIME;
};
void create_coin(glm::vec3 pos);
void update_coin(coin_t& coin);
void delete_coin(coin_t& coin);

struct brick_t {
	int handle = -1;
	static int unbroken_tex_handle;
	static int broken_tex_handle;
	int transform_handle = -1;
	int rec_render_handle = -1;
	int rigidbody_handle = -1;
	glm::vec3 color;
	static const int WIDTH = GAME_GRID_SIZE;
	static const int HEIGHT = GAME_GRID_SIZE;
	static const glm::vec3 BRICK_COLOR;
	bool created_powerup = false;
};

void init_brick_data();
void create_brick(glm::vec3 pos);
void update_brick(brick_t& brick, bool& already_broken);
void delete_brick(brick_t& brick);

struct ice_power_up_t {
	int handle = -1;
	int transform_handle = -1;
	int rec_render_handle = -1;
	int rigidbody_handle = -1;
	float start_y_pos;
	glm::vec3 color;
	static const int WIDTH = 20;
	static const int HEIGHT = 20;
	static const glm::vec3 ICE_POWERUP_COLOR;
	time_count_t creation_time;
};
void create_ice_powerup(glm::vec3 pos);
void update_ice_powerup(ice_power_up_t& power_up);
void delete_ice_powerup_by_kin_handle(int kin_handle);

struct final_flag_t {
	int transform_handle = -1;
	int rec_render_handle = -1;
	int rigidbody_handle = -1;
	static int tex_handle;
	glm::vec3 color;
	static const int WIDTH = GAME_GRID_SIZE;
	static const int HEIGHT = 500;
	static const int RENDER_WIDTH = 30;
	static const int RENDER_HEIGHT = RENDER_WIDTH * 2;
	static glm::vec3 FINAL_FLAG_COLOR;
};

void init_final_flag_data();
void create_final_flag(glm::vec3 pos);
void delete_final_flag();

enum PARALLAX_BCK {
	EVEN1 = 0,
	EVEN2,
	ODD1,
	ODD2,

	NUM_BCKS
};

struct parallax_bck {
	static int transform_handles[NUM_BCKS];
	static int rec_render_handles[NUM_BCKS];
	static int bck_texture;
};
void init_parallax_bck_data();
struct camera_t;
void update_parallax_bcks(camera_t& camera);

struct application_t;
void unload_level(application_t& app);

struct high_scores_t {
	time_count_t times[4]{-1, -1, -1, -1};
};

void init_high_scores(high_scores_t& hs);
void update_high_scores(high_scores_t& hs, time_count_t new_time);