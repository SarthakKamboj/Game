#pragma once

#include "renderer/basic/shape_renders.h"
#include "transform/transform.h"
#include "input/input.h"
#include "physics/physics.h"
#include "glm/glm.hpp"

/**
 * @brief Update all gameobjects
*/
void gos_update();

struct main_character_t {
	// create transform first
	int transform_handle = -1;
	// character currently renders a rectangle, will later change to sprite
	int rec_render_handle = -1;
    // rigidbody for the character
	int rigidbody_handle = -1;
	glm::vec3 color;
	glm::vec2 dims;

	bool grounded = false;
	int num_jumps_since_grounded = 0;
	bool dead = false;

	void update(input::user_input_t& user_input);
};

main_character_t create_main_character(const glm::vec3& pos, const glm::vec3& scale, float rot, glm::vec3& color, const glm::vec2& dims);

struct ground_block_t {
	// create transform first
	int transform_handle = -1;
	// block currently renders a rectangle, will later change to sprite
	int rec_render_handle = -1;
	int rigidbody_handle = -1;
	glm::vec3 color;
	static const int WIDTH = 40;
	static const int HEIGHT = 40;
	static const glm::vec3 BLOCK_COLOR;

    static int tex_handle;
};

ground_block_t create_ground_block(const glm::vec3& pos, const glm::vec3& scale, float rot);

typedef glm::vec3 goomba_turn_pt_t;

/**
 * @brief Represents a Goomba
*/
struct goomba_t {
	int transform_handle = -1;
	int rec_render_handle = -1;
	int rigidbody_handle = -1;
	static const int WIDTH = 40;
	static const int HEIGHT = 40;	
	static const glm::vec3 GOOMBA_COLOR;
	float move_speed = -50.f;
};

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
