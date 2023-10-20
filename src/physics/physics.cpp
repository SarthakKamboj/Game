#include "physics.h"
#include <vector>
#include "constants.h"
#include "transform/transform.h"
#include <iostream>
#include "input/input.h"
#include "utils/time.h"

std::vector<rigidbody_t> non_kin_rigidbodies;
std::vector<rigidbody_t> kin_rigidbodies;

extern int player_rb;

static int num_sat_cols_done;

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

	if (is_kinematic) {
		kin_rigidbodies.push_back(rigidbody);
	}
	else {
		non_kin_rigidbodies.push_back(rigidbody);
	}
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

void rigidbody_t::get_corners(glm::vec2 corners[4]) {
	glm::vec2& top_left = corners[0];
	glm::vec2& top_right = corners[1];
	glm::vec2& bottom_right = corners[2];
	glm::vec2& bottom_left = corners[3];
	transform_t* transform = get_transform(transform_handle);
	assert(transform);
	glm::vec3& pos = transform->position;
	float x_extent = abs(aabb_collider.width / 2);
	float y_extent = abs(aabb_collider.height / 2);
	top_left = glm::vec2(pos.x - x_extent, pos.y + y_extent);
	top_right = glm::vec2(pos.x + x_extent, pos.y + y_extent);
	bottom_left = glm::vec2(pos.x - x_extent, pos.y - y_extent);
	bottom_right = glm::vec2(pos.x + x_extent, pos.y - y_extent);
}

glm::vec2 get_normal(glm::vec2& corner1, glm::vec2& corner2) {
	glm::vec2 dir = corner2 - corner1;
	if (dir.y == 0) return glm::vec2(0, 1);
	return glm::vec2(1, -dir.x / dir.y);
}

bool sat_overlap(float min1, float max1, float min2, float max2) {
	return (min1 >= min2 && max1 <= max2) || (min1 <= min2 && max1 >= max2) || (max1 >= min2 && max1 <= max2) || (min1 >= min2 && min1 <= max2);
}

bool sat_detect_collision(rigidbody_t& rb1, rigidbody_t& rb2) {

	transform_t* transform1 = get_transform(rb1.transform_handle);
	transform_t* transform2 = get_transform(rb2.transform_handle);

	float grid1_x = floor(transform1->position.x / 40);
	float grid1_y = floor(transform1->position.y / 40);

	float grid2_x = floor(transform2->position.x / 40);
	float grid2_y = floor(transform2->position.y / 40);

	if (abs(grid2_x - grid1_x) >= 2 || abs(grid2_y - grid1_y) >= 2) return false;

	num_sat_cols_done++;

	glm::vec2 rb1_corners[4];
	glm::vec2 rb2_corners[4];
	rb1.get_corners(rb1_corners);
	rb2.get_corners(rb2_corners);

	glm::vec2* rbs_corners[2] = {rb1_corners, rb2_corners};
	for (int j = 0; j < 1; j++) {
		glm::vec2* rb_corners = rbs_corners[j];
		for (int i = 0; i < 4; i++) {
			glm::vec2& corner1 = rb_corners[i];
			glm::vec2& corner2 = rb_corners[(i+1)%4];
			glm::vec2 normal = get_normal(corner1, corner2);

			float mins[2] = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
			float maxs[2] = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };
			for (int rb_i = 0; rb_i < 2; rb_i++) {
				for (int corner_i = 0; corner_i < 4; corner_i++) {
					glm::vec2& corner = rbs_corners[rb_i][corner_i];
					float cast = corner.x * normal.x + corner.y * normal.y;
					mins[rb_i] = fmin(mins[rb_i], cast);
					maxs[rb_i] = fmax(maxs[rb_i], cast);
				}
			}

			if (!sat_overlap(mins[0], maxs[0], mins[1], maxs[1])) {
				return false;
			}
		}
	}
	return true;
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

#if 0
	if (col_1_left > col_2_right || col_2_left > col_1_right) return;
	if (col_2_top < col_1_bottom || col_1_top < col_2_bottom) return;
#else	
	if (!sat_detect_collision(rb1, rb2)) return;
#endif

#if 0
	if (player_rb == rb1.handle || player_rb == rb2.handle) {
		int a = 5;
	}
	sat_detect_collision(rb1, rb2);
#endif

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
	for (rigidbody_t& rb : non_kin_rigidbodies) {
        transform_t* ptr = get_transform(rb.transform_handle);
        assert(ptr != NULL);
		transform_t& transform = *ptr;
		if (rb.use_gravity) {
			rb.vel.y -= GRAVITY * platformer::time_t::delta_time;
		}
	}

    // deal with collision from multiple rigidbodies
    // TODO: look into quadtree algorithm to optimize collision detection
	for (int i = 0; i < non_kin_rigidbodies.size(); i++) {
		for (int j = 0; j < kin_rigidbodies.size(); j++) {
			rigidbody_t& rb1 = non_kin_rigidbodies[i];
			rigidbody_t& rb2 = kin_rigidbodies[j];
			handle_collision(rb1, rb2);
		}
	}

    // actually update position of the rigidbody
	for (int i = 0; i < non_kin_rigidbodies.size() + kin_rigidbodies.size(); i++) {
		rigidbody_t* rb_ptr = NULL;
		if (i >= non_kin_rigidbodies.size()) {
			rb_ptr = &kin_rigidbodies[i - non_kin_rigidbodies.size()];
		}
		else {
			rb_ptr = &non_kin_rigidbodies[i];
		}
		assert(rb_ptr);
		rigidbody_t& rb = *rb_ptr;
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
    for (rigidbody_t& rb : kin_rigidbodies) {
        if (rb.handle == rb_handle) {
            return &rb;
        }
    }
	for (rigidbody_t& rb : non_kin_rigidbodies) {
        if (rb.handle == rb_handle) {
            return &rb;
        }
    }
    return NULL;
}
