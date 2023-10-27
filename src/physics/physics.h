#pragma once

#include "renderer/basic/shape_renders.h"

/**
 * @brief AABB collider
*/
struct aabb_collider_t {
	float x = -1.f, y = -1.f;
	float width = -1.f;
	float height = -1.f;

	// for debugging
	int collider_debug_render_handle;
	int collider_debug_transform_handle;
};

struct rigidbody_t {
    int handle = -1;

	aabb_collider_t aabb_collider;
	glm::vec2 vel = glm::vec2(0.f, 0.f);
	int transform_handle = -1;
	bool use_gravity:1;
	bool is_kinematic:1;
	bool debug:1;

	/**
	 * @brief Get the corners for this particular rigidbody
	 * @param corners Input array to store the coordinates of the corners in order top left, top right, bottom right, bottom left
	*/
	void get_corners(glm::vec2 corners[4]);
};

/**
 * @brief Enum is specify collision direction
*/
enum PHYSICS_COLLISION_DIR: uint8_t {
	NONE,
	VERTICAL,
	HORIZONTAL
};

/**
 * @brief The collision detection info for a particular diagnal. It stores the closest ratio to the center
 * where a collision occurred, its corresponding resolving displacement, and the direction of that collision
*/
struct diag_col_info_t {
	float ratio_from_center = 1.f;
	glm::vec2 displacement = glm::vec2(0.f);
	PHYSICS_COLLISION_DIR dir = PHYSICS_COLLISION_DIR::NONE;
};

/**
 * @brief The collision detection info which is basically collision detection for the four diagnals of the
 * object being collided
*/
struct collision_info_t {
	// top left, top right, bottom right, bottom left
	diag_col_info_t diag_cols[4];
};

/// <summary>
/// Detect whether there is a collision between these two rigidbodies using the SAT collision detection algorithm 
/// </summary>
bool sat_detect_collision(rigidbody_t& rb1, rigidbody_t& rb2);

/**
 * @brief Creates a rigidbody for the physics simulation and an AABB box for it as well
 * @param transform_handle The transform associated with the gameobject this rigidbody will be attaching to
 * @param use_gravity Whether it uses gravity or not
 * @param collider_width Width of the AABB collider
 * @param collider_height Height of the AABB collider
 * @param is_kinematic Whether it is kinematic (stationary) or not
 * @return The handle to the newly created rigidbody
*/
int create_rigidbody(int transform_handle, bool use_gravity, float collider_width, float collider_height, bool is_kinematic);

void update_rigidbodies();

/**
 * @brief Get a rigidbody by handle
 * @param rb_handle The rigidbody's handle
*/
rigidbody_t* get_rigidbody(int rb_handle);
