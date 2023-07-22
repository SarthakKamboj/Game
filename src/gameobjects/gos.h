#pragma once

#include "renderer/basic/shape_renders.h"
#include "transform/transform.h"
#include "input/input.h"
#include "physics/physics.h"

struct main_character_t {
	// create transform first
	int transform_handle = -1;
	// character currently renders a rectangle, will later change to sprite
	int rec_render_handle = -1;
    // rigidbody for the character
	int rigidbody_handle = -1;
	glm::vec3 color;
	glm::vec2 dims;
};

main_character_t create_main_character(const glm::vec3& pos, const glm::vec3& scale, float rot, glm::vec3& color, const glm::vec2& dims);
void update_main_character(const main_character_t& mc, key_state_t& key_state);

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
