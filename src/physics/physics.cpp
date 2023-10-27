#include "physics.h"
#include <vector>
#include "constants.h"
#include "transform/transform.h"
#include <iostream>
#include "input/input.h"
#include "utils/time.h"
#include "utils/math.h"

std::vector<rigidbody_t> non_kin_rigidbodies;
std::vector<rigidbody_t> kin_rigidbodies;
std::vector<general_collision_info_t> general_frame_col_infos;

int create_rigidbody(int transform_handle, bool use_gravity, float collider_width, float collider_height, bool is_kinematic, PHYSICS_RB_TYPE rb_type) {
    static int running_count = 0; 

	rigidbody_t rigidbody;
	rigidbody.use_gravity = use_gravity;
	rigidbody.transform_handle = transform_handle;
	rigidbody.is_kinematic = is_kinematic;
    rigidbody.handle = running_count;
	rigidbody.rb_type = rb_type;
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

void rigidbody_t::get_corners(glm::vec2 corners[4]) {
	glm::vec2& top_left = corners[0];
	glm::vec2& top_right = corners[1];
	glm::vec2& bottom_right = corners[2];
	glm::vec2& bottom_left = corners[3];
	
	glm::vec2 pos(aabb_collider.x, aabb_collider.y);

	float x_extent = abs(aabb_collider.width / 2);
	float y_extent = abs(aabb_collider.height / 2);
	top_left = glm::vec2(pos.x - x_extent, pos.y + y_extent);
	top_right = glm::vec2(pos.x + x_extent, pos.y + y_extent);
	bottom_left = glm::vec2(pos.x - x_extent, pos.y - y_extent);
	bottom_right = glm::vec2(pos.x + x_extent, pos.y - y_extent);
}

/**
 * @brief Get the normal vector to a line segment with two corners
 * @param corner1 The first corner of the line segment
 * @param corner2 The second corner of the line segment
*/
glm::vec2 get_normal(glm::vec2& corner1, glm::vec2& corner2) {
	glm::vec2 dir = corner2 - corner1;
	if (dir.y == 0) return glm::vec2(0, 1);
	return glm::vec2(1, -dir.x / dir.y);
}

/**
 * @brief Returns whether given the mins and maxs of projected polygons being compared for SAT collision detection, 
 * there is a overlap in these projections
 * @param min1 Minimum projection of the first polygon
 * @param max1 Maximum projection of the first polygon
 * @param min2 Minimum projection of the second polygon
 * @param max2 Maximum projection of the second polygon
 * @return Whether there was overlap
*/
bool sat_overlap(float min1, float max1, float min2, float max2) {
	bool no_overlap = (min1 > max2) || (min2 > max1);
	return !no_overlap;
}

/**
 * @brief Determines whether there is a collision being two rigidbodies using SAT
 * @param rb1 Rigidbody 1 for collision detection
 * @param rb2 Rigidbody 2 for collision detection
 * @return Whether there is a collision
*/
bool sat_detect_collision(rigidbody_t& rb1, rigidbody_t& rb2) {
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

/**
 * @brief Given a kinematic and non-kinematic rigidbody, determines the collision statuses of the 4 diagnals of the
 * non-kinematic rigidbody with the kinematic rigidbody and updates the col_info struct for a particular diagnal
 * if it has a collision closer to the center than what has already been calculated or defaulted
 * @param kin_rb Kinematic rb
 * @param non_kin_rb Non kinematic rb (such as the player)
 * @param total_col_info Running collision info of the 4 diagnals with this non kinematic rb
 * @param col_info Set collision info for this particular resolving, different from total_col_info because total_col_info is
 * a running record of collision for a non_kin rb with all kin_rbs, this is just for this particular non_kin and kin rb resolving
 * @return Whether there was any sort of collision in this collision resolving
*/
bool diagnals_method_col_resolve(rigidbody_t& kin_rb, rigidbody_t& non_kin_rb, collision_info_t& total_col_info, collision_info_t& cur_col_info) {
	glm::vec2 kin_corners[4];
	glm::vec2 non_kin_corners[4];

	bool detected_col = false;

	kin_rb.get_corners(kin_corners);
	non_kin_rb.get_corners(non_kin_corners);

	for (int non_kin_i = 0; non_kin_i < 4; non_kin_i++) {
		glm::vec2& non_kin_corner = non_kin_corners[non_kin_i];
		glm::vec2 non_kin_center = glm::vec2(non_kin_rb.aabb_collider.x, non_kin_rb.aabb_collider.y);
		float non_kin_slope = (non_kin_corner.y - non_kin_center.y) / (non_kin_corner.x - non_kin_center.x);

		diag_col_info_t& total_diag_col = total_col_info.diag_cols[non_kin_i];
		diag_col_info_t& cur_diag_col = cur_col_info.diag_cols[non_kin_i];

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
					if (ratio_from_center <= total_diag_col.ratio_from_center) {
						float move_ratio = 1 - ratio_from_center;
						transform_t* t_ptr = get_transform(non_kin_rb.transform_handle);
						assert(t_ptr != NULL);
						transform_t& transform = *t_ptr;
						glm::vec2 displacement = -(non_kin_corner - non_kin_center) * move_ratio;
						total_diag_col.dir = HORIZONTAL;
						total_diag_col.ratio_from_center = ratio_from_center;
						total_diag_col.displacement = glm::vec2(displacement.x, 0.f);

						cur_diag_col.dir = HORIZONTAL;
						cur_diag_col.ratio_from_center = ratio_from_center;
						cur_diag_col.displacement = displacement;

						detected_col = true;
					}
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
					// non_kin_rb.vel.y = 0;
					if (ratio_from_center <= total_diag_col.ratio_from_center) {
						float move_ratio = 1 - ratio_from_center;
						transform_t* t_ptr = get_transform(non_kin_rb.transform_handle);
						assert(t_ptr != NULL);
						transform_t& transform = *t_ptr;
						glm::vec2 displacement = -(non_kin_corner - non_kin_center) * move_ratio;
						total_diag_col.dir = VERTICAL;
						total_diag_col.ratio_from_center = ratio_from_center;
						total_diag_col.displacement = glm::vec2(0.f, displacement.y);

						cur_diag_col.dir = VERTICAL;
						cur_diag_col.ratio_from_center = ratio_from_center;
						cur_diag_col.displacement = displacement;

						detected_col = true;
					}
				}
			}

		}
	}
	return detected_col;
}

void update_rigidbodies() {	

	general_frame_col_infos.clear();

	for (rigidbody_t& non_kin_rb_orig : non_kin_rigidbodies) {

		transform_t* ptr = get_transform(non_kin_rb_orig.transform_handle);
		assert(ptr != NULL);
		transform_t& transform_orig = *ptr;

		// glm::vec2 total_displacement(0);
		int num_displacements_x = 0;
		int num_displacements_y = 0;

		const int NUM_CONT_DETECTIONS = 1;
		int earliest_cont_percent_i = NUM_CONT_DETECTIONS;
		PHYSICS_COLLISION_DIR last_valid_dir = PHYSICS_COLLISION_DIR::NONE;

		collision_info_t total_col_info;

		for (int j = 0; j < kin_rigidbodies.size(); j++) {

			rigidbody_t& kin_rb = kin_rigidbodies[j];

			for (int cont_percent_i = 1; cont_percent_i <= earliest_cont_percent_i; cont_percent_i++) {

				time_count_t delta_time = platformer::time_t::delta_time * cont_percent_i / static_cast<float>(NUM_CONT_DETECTIONS);

				// copies of non kinematic rb and transform for this sim at this time delta
				rigidbody_t non_kin_rb = non_kin_rb_orig;
				transform_t transform = transform_orig;

				// apply gravity
				if (non_kin_rb.use_gravity) {
					non_kin_rb.vel.y -= GRAVITY * delta_time;
				}

				transform.position.y += non_kin_rb.vel.y * delta_time;
				transform.position.x += non_kin_rb.vel.x * delta_time;

				non_kin_rb.aabb_collider.x = transform.position.x;
				non_kin_rb.aabb_collider.y = transform.position.y;

				if (!sat_detect_collision(non_kin_rb, kin_rb)) continue;

				collision_info_t cur_col_info;
				bool detected_col = diagnals_method_col_resolve(kin_rb, non_kin_rb, total_col_info, cur_col_info);

				if (detected_col) {
					earliest_cont_percent_i = cont_percent_i;
					for (int i = 0; i < 4; i++) {
						PHYSICS_COLLISION_DIR dir = cur_col_info.diag_cols[i].dir;
						if (dir != PHYSICS_COLLISION_DIR::NONE) {
							PHYSICS_RELATIVE_DIR rel_dir = PHYSICS_RELATIVE_DIR::NONE;
							if (dir == PHYSICS_COLLISION_DIR::HORIZONTAL) {
								if (cur_col_info.diag_cols[i].displacement.x <= 0.f) {
									rel_dir = PHYSICS_RELATIVE_DIR::RIGHT;
								}
								else {
									rel_dir = PHYSICS_RELATIVE_DIR::LEFT;
								}
							}
							else {
								if (cur_col_info.diag_cols[i].displacement.y <= 0.f) {
									rel_dir = PHYSICS_RELATIVE_DIR::TOP;
								}
								else {
									rel_dir = PHYSICS_RELATIVE_DIR::BOTTOM;
								}
							} 
							general_collision_info_t general_col_info(non_kin_rb.rb_type, kin_rb.rb_type, dir, rel_dir);
							general_frame_col_infos.push_back(general_col_info);
							break;
						}
					}
					break;
				}
			}
		}

		time_count_t delta_time = platformer::time_t::delta_time * earliest_cont_percent_i / static_cast<float>(NUM_CONT_DETECTIONS);

		float orig_y = transform_orig.position.y;

		non_kin_rb_orig.vel.y -= GRAVITY * delta_time;
		transform_orig.position.y += non_kin_rb_orig.vel.y * delta_time;
		transform_orig.position.x += non_kin_rb_orig.vel.x * delta_time;

		glm::vec3 total_displacement(0.f);
		for (int i = 0; i < 4; i++) {
			diag_col_info_t& diag_col_info = total_col_info.diag_cols[i];
			if (diag_col_info.dir == PHYSICS_COLLISION_DIR::NONE) continue;
			if (diag_col_info.dir == PHYSICS_COLLISION_DIR::HORIZONTAL && fabs(diag_col_info.displacement.x) >= total_displacement.x) {
				total_displacement.x = diag_col_info.displacement.x;
				non_kin_rb_orig.vel.x = 0;
			}
			else if (diag_col_info.dir == PHYSICS_COLLISION_DIR::VERTICAL && fabs(diag_col_info.displacement.y) >= total_displacement.y){
				total_displacement.y = diag_col_info.displacement.y;
				non_kin_rb_orig.vel.y = 0;
			}
		}

		transform_orig.position += total_displacement;

		non_kin_rb_orig.aabb_collider.x = transform_orig.position.x;
		non_kin_rb_orig.aabb_collider.y = transform_orig.position.y;

		// debugging collider
        transform_t* col_transform_ptr = get_transform(non_kin_rb_orig.aabb_collider.collider_debug_transform_handle);
        assert(col_transform_ptr != NULL);
		transform_t& collider_debug_transform = *col_transform_ptr;
		collider_debug_transform.position.x = non_kin_rb_orig.aabb_collider.x;
		collider_debug_transform.position.y = non_kin_rb_orig.aabb_collider.y;
	}
}

/**
 * @brief Get a rigidbody given a handle
 * @param rb_handle The handle of the rigidbody
 * @return Rigidbody with that handle
*/
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


general_collision_info_t::general_collision_info_t(PHYSICS_RB_TYPE _non_kin_type, PHYSICS_RB_TYPE _kin_type, PHYSICS_COLLISION_DIR _dir, PHYSICS_RELATIVE_DIR _rel_dir) {
	non_kin_type = _non_kin_type;
	kin_type = _kin_type;
	dir = _dir;
	rel_dir = _rel_dir;
}

std::vector<general_collision_info_t> get_general_cols_for_non_kin_type(PHYSICS_RB_TYPE non_kin_type) {
	std::vector<general_collision_info_t> col_infos;
	for (general_collision_info_t& col_info : general_frame_col_infos) {
		if (col_info.non_kin_type == non_kin_type) {
			col_infos.push_back(col_info);
		}
	}
	return col_infos;
}
