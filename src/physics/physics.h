#pragma once

#include "renderer/basic/shape_renders.h"

struct aabb_collider_t {
	float x = -1.f, y = -1.f;
	float width = -1.f;
	float height = -1.f;

	// for debugging
	int collider_debug_render_handle;
	int collider_debug_transform_handle;

	// aabb_collider_t();
};

struct rigidbody_t {
    int handle = -1;

	aabb_collider_t aabb_collider;
	glm::vec2 vel = glm::vec2(0.f, 0.f);
	int transform_handle = -1;
	bool use_gravity:1;
	bool is_kinematic:1;
	bool debug:1;

	// rigidbody_t();
};

enum PHYSICS_COLLISION_DIR: uint8_t {
	NONE,
	VERTICAL,
	HORIZONTAL
};

struct collision_info_t {
	PHYSICS_COLLISION_DIR dir = PHYSICS_COLLISION_DIR::NONE;
	bool is_colliding = false;

	// collision_info_t();
};

int create_rigidbody(int transform_handle, bool use_gravity, float collider_width, float collider_height, bool is_kinematic);
void update_rigidbodies();
rigidbody_t* get_rigidbody(int rb_handle);
