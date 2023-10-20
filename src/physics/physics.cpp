#include "physics.h"
#include <vector>
#include "constants.h"
#include "transform/transform.h"
#include <iostream>
#include "input/input.h"
#include "utils/time.h"

std::vector<rigidbody_t> rigidbodies;

int create_rigidbody(int transform_handle, bool use_gravity, float collider_width, float collider_height, bool is_kinematic) {
    static int running_count = 0; 

	rigidbody_t rigidbody;
	rigidbody.use_gravity = use_gravity;
	rigidbody.transform_handle = transform_handle;
	rigidbody.is_kinematic = is_kinematic;
    rigidbody.handle = running_count;
    running_count++;
	transform_t& transform = *get_transform(transform_handle);

	aabb_collider_t aabb_collider;
	aabb_collider.x = transform.position.x;
	aabb_collider.y = transform.position.y;
	aabb_collider.width = collider_width;
	aabb_collider.height = collider_height;

	// debug stuff
	aabb_collider.collider_debug_transform_handle = create_transform(glm::vec3(aabb_collider.x, aabb_collider.y, 0.f), glm::vec3(1.f), 0.f);
	glm::vec3 collider_color(0.f, 1.f, 0.f);
	aabb_collider.collider_debug_render_handle = create_quad_render(aabb_collider.collider_debug_transform_handle, collider_color, collider_width, collider_height, true, 0, -1);

	rigidbody.aabb_collider = aabb_collider;

	rigidbodies.push_back(rigidbody);
	return rigidbody.handle;
}

void handle_position(rigidbody_t& kinematic_rb, rigidbody_t& non_kinematic_rb, PHYSICS_COLLISION_DIR col_dir) {
    // already verified at this pt that kin_transform and non_kin_transform both exist
	transform_t& kin_transform = *get_transform(kinematic_rb.transform_handle);
	transform_t& non_kin_transform = *get_transform(non_kinematic_rb.transform_handle);

	const float offset = 0;
	if (col_dir == PHYSICS_COLLISION_DIR::HORIZONTAL) {
		if (kin_transform.position.x > non_kin_transform.position.x) {
			non_kin_transform.position.x = kin_transform.position.x - kinematic_rb.aabb_collider.width/2 - non_kinematic_rb.aabb_collider.width/2 + offset;
		}
		else {
			non_kin_transform.position.x = kin_transform.position.x + kinematic_rb.aabb_collider.width/2 + non_kinematic_rb.aabb_collider.width/2 - offset;
		}
	}
	else if (col_dir == PHYSICS_COLLISION_DIR::VERTICAL) {
		if (kin_transform.position.y > non_kin_transform.position.y) {
			non_kin_transform.position.y = kin_transform.position.y - kinematic_rb.aabb_collider.height/2 - non_kinematic_rb.aabb_collider.height/2 + offset;
		}
		else {
			non_kin_transform.position.y = kin_transform.position.y + kinematic_rb.aabb_collider.height/2 + non_kinematic_rb.aabb_collider.height/2 - offset;
		}
	}
}

void handle_collision(rigidbody_t& rb1, rigidbody_t& rb2) {
	aabb_collider_t& collider_1 = rb1.aabb_collider;
	aabb_collider_t& collider_2 = rb2.aabb_collider;

	float col_1_right = collider_1.x + collider_1.width / 2;
	float col_1_left = collider_1.x - collider_1.width / 2;
	float col_1_top = collider_1.y + collider_1.height / 2;
	float col_1_bottom = collider_1.y - collider_1.height / 2;

	float col_2_right = collider_2.x + collider_2.width / 2;
	float col_2_left = collider_2.x - collider_2.width / 2;
	float col_2_top = collider_2.y + collider_2.height / 2;
	float col_2_bottom = collider_2.y - collider_2.height / 2;

	if (col_1_left > col_2_right || col_2_left > col_1_right) return;
	if (col_2_top < col_1_bottom || col_1_top < col_2_bottom) return;

	// rb1 is on the right and rb2 is on the left
	if (col_1_left == col_2_right) {
		// don't allow rb2 to move right
		if (rb2.vel.x > 0) rb2.vel.x = 0.f;
		// don't allow rb1 to move left
		if (rb1.vel.x < 0) rb1.vel.x = 0.f;
		return;
	}
	// rb2 is on the right and rb1 is on the left
	else if (col_2_left == col_1_right) {
		// don't allow rb2 to move left
		if (rb2.vel.x < 0) rb2.vel.x = 0.f;
		// don't allow rb1 to move right
		if (rb1.vel.x > 0) rb1.vel.x = 0.f;
		return;
	}

	// rb2 on bottom and rb1 on top
	if (col_2_top == col_1_bottom) {
		// don't let rb2 go up
		if (rb2.vel.y > 0) rb2.vel.y = 0.f;
		// don't let rb1 go down
		if (rb1.vel.y < 0) rb1.vel.y = 0.f;
		return;
	}
	// rb1 on bottom and rb2 on top
	else if (col_1_top == col_2_bottom) {
		// don't let rb2 go down
		if (rb2.vel.y < 0) rb2.vel.y = 0.f;
		// don't let rb1 go up
		if (rb1.vel.y > 0) rb1.vel.y = 0.f;
		return;
	}

	collision_info_t col_info;
	col_info.is_colliding = true;
	col_info.dir = VERTICAL;

	float overlap_from_col1_x = col_1_right - col_2_left;
	float overlap_from_col2_x = col_2_right - col_1_left;

	float overlap_from_col1_y = col_1_top - col_2_bottom;
	float overlap_from_col2_y = col_2_top - col_1_bottom;

	bool overlap_on_x = fmin(overlap_from_col1_x, overlap_from_col2_x) < MAX_HORIZONTAL_COL_OFFSET_PIXELS;

    // the overlap in y direction is too much to be considered vertical collision, 
    // rather the 2 rigidbodies are adjacent to each other
	if (overlap_on_x && fmin(overlap_from_col1_y, overlap_from_col2_y) > MAX_PIXELS_OVERLAP_FOR_VERT_COL) {
		col_info.dir = HORIZONTAL;
	}

	if (col_info.dir == VERTICAL) {
		rb1.vel.y = 0.f;
		rb2.vel.y = 0.f;
	}
	else {
		rb1.vel.x = 0.f;
		rb2.vel.x = 0.f;
	}

    transform_t* t1_ptr = get_transform(rb1.transform_handle);
    transform_t* t2_ptr = get_transform(rb2.transform_handle);
    assert(t1_ptr != NULL);
    assert(t2_ptr != NULL);
	transform_t& transform1 = *t1_ptr;
	transform_t& transform2 = *t2_ptr;

	if (rb1.is_kinematic != rb2.is_kinematic) {
		if (rb1.is_kinematic) {
			handle_position(rb1, rb2, col_info.dir);
		}
		else {
			handle_position(rb2, rb1, col_info.dir);
		}
	}
}

void update_rigidbodies() {	

    // apply gravity
	for (rigidbody_t& rb : rigidbodies) {
        transform_t* ptr = get_transform(rb.transform_handle);
        assert(ptr != NULL);
		transform_t& transform = *ptr;
		if (rb.use_gravity) {
			rb.vel.y -= GRAVITY * platformer::time_t::delta_time;
		}
	}

    // deal with collision from multiple rigidbodies
    // TODO: look into quadtree algorithm to optimize collision detection
	for (int i = 0; i < rigidbodies.size(); i++) {
		for (int j = i + 1; j < rigidbodies.size(); j++) {
			rigidbody_t& rb1 = rigidbodies[i];
			rigidbody_t& rb2 = rigidbodies[j];
			handle_collision(rb1, rb2);
		}
	}

    // actually update position of the rigidbody
	for (rigidbody_t& rb : rigidbodies) {
        transform_t* ptr = get_transform(rb.transform_handle);
        assert(ptr != NULL);
		transform_t& transform = *ptr;
		transform.position.y += rb.vel.y * platformer::time_t::delta_time;
		transform.position.x += rb.vel.x * platformer::time_t::delta_time;
		rb.aabb_collider.x = transform.position.x;
		rb.aabb_collider.y = transform.position.y;

		// debugging collider
        transform_t* col_transform_ptr = get_transform(rb.aabb_collider.collider_debug_transform_handle);
        assert(col_transform_ptr != NULL);
		transform_t& collider_debug_transform = *col_transform_ptr;
		collider_debug_transform.position.x = rb.aabb_collider.x;
		collider_debug_transform.position.y = rb.aabb_collider.y;
	}	
}

rigidbody_t* get_rigidbody(int rb_handle) {
    for (rigidbody_t& rb : rigidbodies) {
        if (rb.handle == rb_handle) {
            return &rb;
        }
    }
    return NULL;
}
