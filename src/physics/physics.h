#pragma once

#include "renderer/basic/shape_renders.h"
#include <vector>

/**
 * @brief The type of rigidbody this will be, these are more game specific
*/
enum class PHYSICS_RB_TYPE {
	NONE,
	PLAYER,
	GROUND,
	BRICK
};

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

	PHYSICS_RB_TYPE rb_type = PHYSICS_RB_TYPE::NONE;
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

enum class PHYSICS_RELATIVE_DIR {
	NONE,
	RIGHT,
	LEFT,
	TOP,
	BOTTOM
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
 * @param rb_type What category this rigidbody is associated with
 * @return The handle to the newly created rigidbody
*/
int create_rigidbody(int transform_handle, bool use_gravity, float collider_width, float collider_height, bool is_kinematic, PHYSICS_RB_TYPE rb_type);

/**
 * @brief Iterates over every non kinematic rb and performs collision detection with every kinematic rb.
 * Optimizes collision detection to ensure two items are in the same grid to avoid unnecessary collision
 * calculations. Also will do continuous collision detection in the form of splitting delta time into smaller
 * chunks for more granular testing but this will most likely be removed and unnecessary at this point. The
 * collision detection is done separately per diagnal per non-kinematic rb where each diagnal stores its highest
 * penetrating collision. After iterating over all kinematic rbs, the diagnals' collision detection information
 * is looked through to calculate the highest x and y displacement to resolve the position and the position is
 * displaced accordingly. Also stores per frame information about collisions that occurred.
*/
void update_rigidbodies();

/**
 * @brief Get a rigidbody by handle
 * @param rb_handle The rigidbody's handle
*/
rigidbody_t* get_rigidbody(int rb_handle);

/**
 * @brief General collision information about non kin rb type, kin rb type, relative direction of kin rb relative to
 * non_kin rb, and dir of collision
*/
struct general_collision_info_t {
	PHYSICS_RB_TYPE non_kin_type = PHYSICS_RB_TYPE::NONE;
	PHYSICS_RB_TYPE kin_type = PHYSICS_RB_TYPE::NONE;
	PHYSICS_COLLISION_DIR dir = PHYSICS_COLLISION_DIR::NONE;
	PHYSICS_RELATIVE_DIR rel_dir = PHYSICS_RELATIVE_DIR::NONE;

	general_collision_info_t(PHYSICS_RB_TYPE non_kin_type, PHYSICS_RB_TYPE kin_type, PHYSICS_COLLISION_DIR dir, PHYSICS_RELATIVE_DIR rel_dir);
};

/**
 * @brief Get the general collision information for all collisions for a particular non_kin rb type this frame
 * @param non_kin_type The type of the non_kin
 * @return All collisions for a particular non_kin rb type this frame (note this will be separate for different diagnals so if a
 * non_kin rb and kin rb have 2 collisions on 2 different diagnals, it records them both...this can be changed later if need be)
*/
std::vector<general_collision_info_t> get_general_cols_for_non_kin_type(PHYSICS_RB_TYPE non_kin_type);
