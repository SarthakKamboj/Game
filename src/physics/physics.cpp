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
	bool no_overlap = (min1 >= max2) || (min2 >= max1);
	return !no_overlap;
	// return (min1 >= min2 && max1 <= max2) || (min1 <= min2 && max1 >= max2) || (max1 >= min2 && max1 <= max2) || (min1 >= min2 && min1 <= max2);
}

bool sat_detect_collision(rigidbody_t& rb1, rigidbody_t& rb2) {

	// transform_t* transform1 = get_transform(rb1.transform_handle);
	// transform_t* transform2 = get_transform(rb2.transform_handle);
	transform_t transform1_obj;
	transform1_obj.position.x = rb1.aabb_collider.x;
	transform1_obj.position.y = rb1.aabb_collider.y;
	transform1_obj.position.z = 0;
	transform_t* transform1 = &transform1_obj;

	transform_t transform2_obj;
	transform2_obj.position.x = rb2.aabb_collider.x;
	transform2_obj.position.y = rb2.aabb_collider.y;
	transform2_obj.position.z = 0;
	transform_t* transform2 = &transform2_obj;

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

// void diag_pos_resolve(rigidbody_t& kin_rb, rigidbody_t& non_kin_rb) {
glm::vec2 diag_pos_resolve(rigidbody_t& kin_rb, rigidbody_t& non_kin_rb, collision_info_t& col_info) {
	glm::vec2 kin_corners[4];
	glm::vec2 non_kin_corners[4];
	col_info.is_colliding = false;
	col_info.dir = NONE;

	kin_rb.get_corners(kin_corners);
	non_kin_rb.get_corners(non_kin_corners);

	for (int non_kin_i = 0; non_kin_i < 4; non_kin_i++) {
		glm::vec2& non_kin_corner = non_kin_corners[non_kin_i];
		glm::vec2 non_kin_center = glm::vec2(non_kin_rb.aabb_collider.x, non_kin_rb.aabb_collider.y);
		float non_kin_slope = (non_kin_corner.y - non_kin_center.y) / (non_kin_corner.x - non_kin_center.x);

		for (int kin_i = 0; kin_i < 4; kin_i++) {
			glm::vec2& corner1 = kin_corners[kin_i];
			glm::vec2& corner2 = kin_corners[(kin_i+1)%4];

			if (corner1.x == corner2.x) {
				float y_point_on_non_kin_diag = non_kin_slope * (corner1.x - non_kin_center.x) + non_kin_center.y;
				glm::vec2 intersection_pt(corner1.x, y_point_on_non_kin_diag);
				float ratio_from_center_numerator = glm::pow(intersection_pt.x - non_kin_center.x, 2) + glm::pow(intersection_pt.y - non_kin_center.y, 2);
				float ratio_from_center_denom = glm::pow(non_kin_corner.x - non_kin_center.x, 2) + glm::pow(non_kin_corner.y - non_kin_center.y, 2);
				float dir = glm::dot(glm::normalize(intersection_pt - non_kin_center), glm::normalize(non_kin_corner - non_kin_center));
				float ratio_from_center = glm::sqrt(ratio_from_center_numerator / ratio_from_center_denom);
				if (dir >= 0 && ratio_from_center >= 0 && ratio_from_center <= 1.f && intersection_pt.y >= fmin(corner1.y, corner2.y) && intersection_pt.y <= fmax(corner1.y, corner2.y)) {
					non_kin_rb.vel.x = 0;
					float move_ratio = 1 - ratio_from_center;
					transform_t* t_ptr = get_transform(non_kin_rb.transform_handle);
					assert(t_ptr != NULL);
					transform_t& transform = *t_ptr;
					glm::vec2 displacement = -(non_kin_corner - non_kin_center) * move_ratio;
					col_info.is_colliding = true;
					col_info.dir = HORIZONTAL;
					return displacement;
					// transform.position -= glm::vec3(displacement, 0);
				}
			}
			else if (corner1.y == corner2.y) {
				float x_point_on_non_kin_diag = ((corner1.y - non_kin_center.y) / non_kin_slope) + non_kin_center.x;
				glm::vec2 intersection_pt(x_point_on_non_kin_diag, corner1.y);
				float ratio_from_center_numerator = glm::pow(intersection_pt.x - non_kin_center.x, 2) + glm::pow(intersection_pt.y - non_kin_center.y, 2);
				float ratio_from_center_denom = glm::pow(non_kin_corner.x - non_kin_center.x, 2) + glm::pow(non_kin_corner.y - non_kin_center.y, 2);
				float dir = glm::dot(glm::normalize(intersection_pt - non_kin_center), glm::normalize(non_kin_corner - non_kin_center));
				float ratio_from_center = glm::sqrt(ratio_from_center_numerator / ratio_from_center_denom);
				if (dir >= 0 && ratio_from_center >= 0 && ratio_from_center <= 1.f && intersection_pt.x >= fmin(corner1.x, corner2.x) && intersection_pt.x <= fmax(corner1.x, corner2.x)) {
					non_kin_rb.vel.y = 0;
					float move_ratio = 1 - ratio_from_center;
					transform_t* t_ptr = get_transform(non_kin_rb.transform_handle);
					assert(t_ptr != NULL);
					transform_t& transform = *t_ptr;
					glm::vec2 displacement = -(non_kin_corner - non_kin_center) * move_ratio;
					col_info.is_colliding = true;
					col_info.dir = VERTICAL;
					return displacement;
					// transform.position -= glm::vec3(displacement, 0);
				}
			}

		}
	}
	return glm::vec2(0);
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
	// if (!sat_detect_collision(rb1, rb2)) return;
#endif

#if 1
	if (player_rb == rb1.handle || player_rb == rb2.handle) {
		int a = 5;
	}
	// sat_detect_collision(rb1, rb2);
#endif

#if 1

#if 0
	if (rb1.is_kinematic != rb2.is_kinematic) {
		if (rb1.is_kinematic) {
			diag_pos_resolve(rb1, rb2);
		}
		else {
			diag_pos_resolve(rb2, rb1);
		}
	}
#endif

	collision_info_t col_info;
	col_info.is_colliding = true;
	col_info.dir = VERTICAL;

	float overlap_from_col1_x = col_1_right - col_2_left;
	float overlap_from_col2_x = col_2_right - col_1_left;

	float overlap_from_col1_y = col_1_top - col_2_bottom;
	float overlap_from_col2_y = col_2_top - col_1_bottom;

	bool overlapping_in_x_dir = fmin(overlap_from_col1_x, overlap_from_col2_x) < MAX_HORIZONTAL_PIXEL_OVERLAP_FOR_HORIZONTAL_COL;

    // the overlap in y direction is too much to be considered vertical collision, 
    // rather the 2 rigidbodies are adjacent to each other
	if (overlapping_in_x_dir && fmin(overlap_from_col1_y, overlap_from_col2_y) > MAX_PIXELS_OVERLAP_FOR_VERT_COL) {
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

#if 0
	// right now this only handle non-kin w/ kin position handling
	if (rb1.is_kinematic != rb2.is_kinematic) {
		if (rb1.is_kinematic) {
			handle_position(rb1, rb2, col_info.dir);
		}
		else {
			handle_position(rb2, rb1, col_info.dir);
		}
	}
#endif

#else
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
#endif
}

void update_rigidbodies() {	

	// SIMULATE THE WORLD

	const float NUM_CONT_DETECTIONS = 1.f;
	for (int cont_percent_i = 1; cont_percent_i <= NUM_CONT_DETECTIONS; cont_percent_i++) {

		float delta_time = platformer::time_t::delta_time * cont_percent_i / NUM_CONT_DETECTIONS;

		// apply gravity
		for (rigidbody_t& non_kin_rb : non_kin_rigidbodies) {
			transform_t* ptr = get_transform(non_kin_rb.transform_handle);
			assert(ptr != NULL);
			transform_t& transform = *ptr;
			if (non_kin_rb.use_gravity) {
				non_kin_rb.vel.y -= GRAVITY * delta_time;
			}

		// actually update position of the rigidbody
		// for (int i = 0; i < non_kin_rigidbodies.size() + kin_rigidbodies.size(); i++) {
#if 0
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
#endif
			transform.position.y += non_kin_rb.vel.y * delta_time;
			transform.position.x += non_kin_rb.vel.x * delta_time;

			non_kin_rb.aabb_collider.x = transform.position.x;
			non_kin_rb.aabb_collider.y = transform.position.y;

			// TODO: look into quadtree algorithm to optimize collision detection
			num_sat_cols_done = 0;
#if 0
			for (int i = 0; i < non_kin_rigidbodies.size(); i++) {
				for (int j = i + 1; j < non_kin_rigidbodies.size(); j++) {
					rigidbody_t& rb1 = non_kin_rigidbodies[i];
					rigidbody_t& rb2 = non_kin_rigidbodies[j];
					if (!sat_detect_collision(rb1, rb2)) return;
					handle_collision(rb1, rb2);
				}
			}
#endif

			glm::vec2 total_displacement(0);
			int num_displacements_x = 0;
			int num_displacements_y = 0;
			// rigidbody_t& rb1 = non_kin_rigidbodies[i];
			collision_info_t col_info;
			PHYSICS_COLLISION_DIR last_valid_dir = NONE;
			for (int j = 0; j < kin_rigidbodies.size(); j++) {
				rigidbody_t& rb2 = kin_rigidbodies[j];
				if (!sat_detect_collision(non_kin_rb, rb2)) continue;
				// displacement should always be more than 0
				glm::vec2 displacement = diag_pos_resolve(rb2, non_kin_rb, col_info);
				if (displacement != glm::vec2(0)) {
					last_valid_dir = col_info.dir;
	#if 0
					if (col_info.dir == HORIZONTAL) {
						int b = 10;
						glm::vec2 displacement = diag_pos_resolve(rb2, rb1, col_info);
					}
					else {
						int b = 10;
						glm::vec2 displacement = diag_pos_resolve(rb2, rb1, col_info);
					}
	#endif

					if (col_info.dir == HORIZONTAL) {
						total_displacement.x += displacement.x;
						num_displacements_x++;
					}
					else if (col_info.dir == VERTICAL) {
						total_displacement.y += displacement.y;
						num_displacements_y++;
					}
				}
				else {
					printf("here");
				}
				// handle_collision(rb1, rb2);
			}
#if 0
			transform_t* t_ptr = get_transform(rb1.transform_handle);
			assert(t_ptr != NULL);
			transform_t& transform = *t_ptr;
#endif
			if (num_displacements_x + num_displacements_y == 1) {
				if (last_valid_dir == VERTICAL) {
					static int i = 0;
					if (i == 1) {
						// printf("here");
					}
					i++;
					transform.position += glm::vec3(0, total_displacement.y, 0);
				}
				else {
					transform.position += glm::vec3(total_displacement.x, 0, 0);
				}
			}
			else if (num_displacements_x + num_displacements_y > 1) {
				// transform.position += glm::vec3((total_displacement / static_cast<float>(num_displacements)), 0);
				transform.position += glm::vec3(total_displacement.x / static_cast<float>(fmax(1, num_displacements_x)), total_displacement.y / static_cast<float>(fmax(1, num_displacements_y)), 0);
			}
			transform.dirty_model_matrix = true;
		}
	}

	// std::cout << "num_sat_cols_done: " << num_sat_cols_done << std::endl;

	// actually update position of the rigidbody
	// for (int i = 0; i < non_kin_rigidbodies.size() + kin_rigidbodies.size(); i++) {

	// update collider and debug collider view positions
	for (int i = 0; i < non_kin_rigidbodies.size(); i++) {
		rigidbody_t& rb = non_kin_rigidbodies[i];
        transform_t* ptr = get_transform(rb.transform_handle);
        assert(ptr != NULL);
		transform_t& transform = *ptr;

		rb.aabb_collider.x = transform.position.x;
		rb.aabb_collider.y = transform.position.y;

		// debugging collider
        transform_t* col_transform_ptr = get_transform(rb.aabb_collider.collider_debug_transform_handle);
        assert(col_transform_ptr != NULL);
		transform_t& collider_debug_transform = *col_transform_ptr;
		collider_debug_transform.dirty_model_matrix = true;
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
