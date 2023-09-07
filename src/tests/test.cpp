
#include <catch2/catch_test_macros.hpp>
#include "test_config.h"
#include "update.h"
#include "transform/transform.h"
#include <iostream>
// #include "networking/networking.h"
// #include "update.h"
// #include "constants.h"

// #include "shared/utils/fifo.h"

// snapshots_fifo_t gameobject_saved_snapshots;

int object_transform_handle;
int player_transform_handle;
extern snapshots_fifo_t snapshot_fifo;

TEST_CASE("Testing basic server res handling for snapshots") {
	world::reset();
	REQUIRE(snapshot_fifo.empty);
	object_transform_handle = create_transform(glm::vec3(0), glm::vec3(1), 0);
	player_transform_handle = -1;

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