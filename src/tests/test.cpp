
#include <catch2/catch_test_macros.hpp>
#include "test_config.h"
#include "update.h"
#include "transform/transform.h"
#include <iostream>
// #include "networking/networking.h"
// #include "update.h"
// #include "constants.h"

// extern bool started_updates;
extern snapshots_fifo_t snapshot_fifo;

int object_transform_handle = -1;
int player_transform_handle = -1;
// int server_object_transform_handle = -1;

// input::user_input_t input_state;

/*
TEST_CASE("Smooth damp tests 2.28") {
	world::smooth_damp_info_t damp_info;
	damp_info.finished = false;
	std::vector<float> targets = { 2.28f, 110.f, 34.56f, -2.28f, -349.52f, -2391.f };
	std::vector<float> ratios = { 0, 0.12f, 0.23f, 0.56f, 0.59f, .68f, .79f, 0.91f, 0.98f };
	for (int t = 0; t < targets.size(); t++) {
		for (int r = 0; r < ratios.size(); r++) {
			damp_info.start_time = 12323901.212309;
			damp_info.total_time = 1.30921;
			platformer::time_t::cur_time = damp_info.start_time + damp_info.total_time * ratios[r];
			// REQUIRE(2.28f == smooth_damp(0, 2.28, damp_info));
			float smoothed = smooth_damp(0, targets[t], damp_info);
			std::cout << "target: " << targets[t] << " ratio: " << ratios[r] << " smoothed: " << smoothed << std::endl;
		}
	}
}
*/

/*
TEST_CASE("Smooth damp tests") {
	world::smooth_damp_info_t damp_info;
	damp_info.finished = false;
	std::vector<float> targets = { -30.f };
	std::vector<float> ratios = { 0, 0.12f, 0.23f, 0.56f, 0.59f, .68f, .79f, 0.91f, 0.98f };
	for (int t = 0; t < targets.size(); t++) {
		for (float i = 0; i < 60; i++) {
			damp_info.start_time = 12323901.212309;
			damp_info.total_time = 1.30921;
			float ratio = i / 60.f;
			platformer::time_t::cur_time = damp_info.start_time + damp_info.total_time * ratio;
			// REQUIRE(2.28f == smooth_damp(0, 2.28, damp_info));
			float smoothed = smooth_damp(0, targets[t], damp_info);
			std::cout << "smoothed: " << smoothed << "   target: " << targets[t] << " ratio: " << ratio << std::endl;
		}
	}
}
*/

/*
TEST_CASE("Smooth damp tests -2.28") {
	world::smooth_damp_info_t damp_info;
	damp_info.finished = false;
	damp_info.start_time = 0;
	damp_info.total_time = 1;
	platformer::time_t::cur_time = damp_info.start_time + damp_info.total_time * 0.98;
	REQUIRE(-2.28f == smooth_damp(0, -2.28, damp_info));
}
*/

TEST_CASE("Testing basic server res handling for snapshots") {
	object_transform_handle = create_transform(glm::vec3(250, 50, 0), glm::vec3(1), 0);
	transform_t* transform_ptr = get_transform(object_transform_handle);

	world::reset();
	world::snapshot_t snapshot_1;
	snapshot_1.game_time = 0;
	snapshot_1.gameobjects[0].x = 200;
	snapshot_1.gameobjects[0].y = 50;
	snapshot_1.snapshot_id = 0;

	world::snapshot_t snapshot_2;
	snapshot_2.game_time = 0.05;
	snapshot_2.gameobjects[0].x = 300;
	snapshot_2.gameobjects[0].y = 20;
	snapshot_2.snapshot_id = 1;
	snapshot_fifo.enqueue(snapshot_2);

	world::obj_update_info_t update_info;
	update_info.snapshot_from = snapshot_1;
	update_info.snapshot_to = &snapshot_fifo.container[0];
	update_info.update_mode = world::OBJECT_UPDATE_MODE::INTERPOLATION;
	update_info.last_frame_update_mode = world::OBJECT_UPDATE_MODE::EXTRAPOLATION;
	update_info.last_extrapolation_time = 0.008;

	platformer::time_t::cur_time = 0.01;

	while (platformer::time_t::cur_time < 0.05) {
		utils::game_timer_t frame_timer;	
		utils::start_timer(frame_timer);
		world::update_interpolated_objs(update_info);
		utils::end_timer(frame_timer);
		// world::update_interpolated_objs(update_info);
		platformer::time_t::delta_time = frame_timer.elapsed_time_sec;
		platformer::time_t::cur_time += frame_timer.elapsed_time_sec;
	}


	// REQUIRE();

	// gameobject_saved_snapshots.reset();
	// REQUIRE(gameobject_saved_snapshots.empty == true);

	// server_res_body_t server_res_body;
	// server_res_body.res_type = RES_TYPE::SNAPSHOT;

	// snapshot_t snapshot;
	// snapshot.game_time = 0;
	// snapshot.snapshot_id = 0;
	// server_res_body.res_data.snapshot_data = snapshot;
	// handle_server_res_body(server_res_body);

	// snapshot.game_time = 0.05;
	// snapshot.snapshot_id = 1;
	// server_res_body.res_data.snapshot_data = snapshot;
	// handle_server_res_body(server_res_body);

	// snapshot.game_time = 0.05;
	// snapshot.snapshot_id = 2;
	// server_res_body.res_data.snapshot_data = snapshot;
	// handle_server_res_body(server_res_body);

	// REQUIRE(gameobject_saved_snapshots.full == true);

	// std::array<unsigned int, 3> snapshot_ids;
	// snapshots_fifo_t::dequeue_state_t dequeue = gameobject_saved_snapshots.dequeue();
	// REQUIRE(dequeue.valid == true);
	// snapshot_ids[0] = dequeue.val.snapshot_id;

	// dequeue = gameobject_saved_snapshots.dequeue();
	// REQUIRE(dequeue.valid == true);
	// snapshot_ids[1] = dequeue.val.snapshot_id;

	// dequeue = gameobject_saved_snapshots.dequeue();
	// REQUIRE(dequeue.valid == true);
	// snapshot_ids[2] = dequeue.val.snapshot_id;

	// REQUIRE(gameobject_saved_snapshots.empty == true);
	// REQUIRE(snapshot_ids == std::array<unsigned int, 3>{0,1,2});
}

// TEST_CASE("Testing server res handling for snapshots out of order part 1") {

// 	gameobject_saved_snapshots.reset();
// 	REQUIRE(gameobject_saved_snapshots.empty == true);

// 	server_res_body_t server_res_body;
// 	server_res_body.res_type = RES_TYPE::SNAPSHOT;

// 	snapshot_t snapshot;
// 	snapshot.game_time = 0.1;
// 	snapshot.snapshot_id = 4;
// 	server_res_body.res_data.snapshot_data = snapshot;
// 	handle_server_res_body(server_res_body);

// 	REQUIRE(gameobject_saved_snapshots.full == false);

// 	snapshot.game_time = 0.1;
// 	snapshot.snapshot_id = 3;
// 	server_res_body.res_data.snapshot_data = snapshot;
// 	handle_server_res_body(server_res_body);

// 	std::array<unsigned int, 3> snapshot_ids;
// 	snapshots_fifo_t::dequeue_state_t dequeud = gameobject_saved_snapshots.dequeue();
// 	REQUIRE(dequeud.valid == true);
// 	snapshot_ids[0] = dequeud.val.snapshot_id;
	
// 	REQUIRE(gameobject_saved_snapshots.empty == true);

// 	dequeud = gameobject_saved_snapshots.dequeue();
// 	REQUIRE(dequeud.valid == false);
// 	snapshot_ids[1] = INVALID_SNAPSHOT_ID;

// 	dequeud = gameobject_saved_snapshots.dequeue();
// 	REQUIRE(dequeud.valid == false);
// 	snapshot_ids[2] = INVALID_SNAPSHOT_ID;

// 	REQUIRE(snapshot_ids == std::array<unsigned int, 3>{4, INVALID_SNAPSHOT_ID, INVALID_SNAPSHOT_ID});
// }

// TEST_CASE("Testing server res handling for snapshots out of order part 2") {

// 	gameobject_saved_snapshots.reset();
// 	REQUIRE(gameobject_saved_snapshots.empty == true);

// 	server_res_body_t server_res_body;
// 	server_res_body.res_type = RES_TYPE::SNAPSHOT;

// 	snapshot_t snapshot;
// 	snapshot.game_time = 0.1;
// 	snapshot.snapshot_id = 0;
// 	server_res_body.res_data.snapshot_data = snapshot;
// 	handle_server_res_body(server_res_body);

// 	REQUIRE(gameobject_saved_snapshots.full == false);

// 	snapshot.game_time = 0.1;
// 	snapshot.snapshot_id = 2;
// 	server_res_body.res_data.snapshot_data = snapshot;
// 	handle_server_res_body(server_res_body);

// 	snapshots_fifo_t::dequeue_state_t dequeud = gameobject_saved_snapshots.dequeue();
// 	REQUIRE(dequeud.valid == true);
// 	REQUIRE(dequeud.val.snapshot_id == 0);

// 	snapshot.game_time = 0.1;
// 	snapshot.snapshot_id = 1;
// 	server_res_body.res_data.snapshot_data = snapshot;
// 	handle_server_res_body(server_res_body);

// 	std::array<unsigned int, 3> snapshot_ids;

// 	dequeud = gameobject_saved_snapshots.dequeue();
// 	REQUIRE(dequeud.valid == true);
// 	snapshot_ids[0] = dequeud.val.snapshot_id;
	
// 	REQUIRE(gameobject_saved_snapshots.empty == true);

// 	dequeud = gameobject_saved_snapshots.dequeue();
// 	REQUIRE(dequeud.valid == false);
// 	snapshot_ids[1] = INVALID_SNAPSHOT_ID;

// 	dequeud = gameobject_saved_snapshots.dequeue();
// 	REQUIRE(dequeud.valid == false);
// 	snapshot_ids[2] = INVALID_SNAPSHOT_ID;

// 	REQUIRE(snapshot_ids == std::array<unsigned int, 3>{2, INVALID_SNAPSHOT_ID, INVALID_SNAPSHOT_ID});
// }