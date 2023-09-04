#include "app.h" 
#include "transform/transform.h"
#include "utils/time.h"
#include "constants.h"

bool assign_interpolating_snapshots(snapshots_fifo_t& snapshot_fifo, update_info_t& update_info) {
	snapshots_fifo_t::dequeue_state_t dequeue_state = snapshot_fifo.dequeue();
	if (dequeue_state.valid) {
		update_info.snapshot_from = dequeue_state.val;
	} else {
		// TODO: this is most likely that we have nothing to even interpolate from or to
		// LEAST IDEAL SITUATION TO BE IN
		std::cout << "dequeue was not valid" << std::endl;
		return false;
	}

	snapshots_fifo_t::peek_state_t peek_state = snapshot_fifo.peek_read();
	if (peek_state.valid) {
		update_info.snapshot_to = peek_state.val;
	}	 else {
		// TODO: this is most likely situation where we do not have anything to interpolate to so we will need to do extrapolation until the next snapshot comes
		std::cout << "peek was not valid" << std::endl;
		// put the from snapshot back in to be used later
		snapshot_fifo.enqueue(update_info.snapshot_from);
		update_info.update_mode = update_mode_t::EXTRAPOLATION;
		return false;
	}

	update_info.update_mode = update_mode_t::INTERPOLATION;
	return true;
}

void handle_snapshots(snapshots_fifo_t& snapshot_fifo, update_info_t& update_info) {
	snapshot_data_t& snapshot_from = update_info.snapshot_from;
	snapshot_data_t*& snapshot_to = update_info.snapshot_to;	

	if (update_info.update_mode == update_mode_t::EXTRAPOLATION) {
		assign_interpolating_snapshots(snapshot_fifo, update_info);
	}

	if (update_info.update_mode == update_mode_t::EXTRAPOLATION) return;
	
	if (platformer::time_t::cur_time >= snapshot_to->game_time) {
		assign_interpolating_snapshots(snapshot_fifo, update_info); 
	}	
}

float remap(float val, float orig_low, float orig_high, float new_low, float new_high) {
	return new_low + (val - orig_low) * ((new_high - new_low) / (orig_high - orig_low));
}

void update_object_positions(update_info_t& update_info, int transform_handle) {
	transform_t* transform_ptr = get_transform(transform_handle);
	assert(transform_ptr != NULL);

	if (update_info.update_mode == update_mode_t::INTERPOLATION) {
		snapshot_data_t& snapshot_from = update_info.snapshot_from;
		snapshot_data_t*& snapshot_to = update_info.snapshot_to;
		float iter_val = (platformer::time_t::cur_time - snapshot_from.game_time) / (snapshot_to->game_time - snapshot_from.game_time);
		transform_ptr->position.x = iter_val * (snapshot_to->gameobject_snapshots[0].x - snapshot_from.gameobject_snapshots[0].x) + snapshot_from.gameobject_snapshots[0].x;
		transform_ptr->position.y = iter_val * (snapshot_to->gameobject_snapshots[0].y - snapshot_from.gameobject_snapshots[0].y) + snapshot_from.gameobject_snapshots[0].y;
	} else if (update_info.update_mode == update_mode_t::EXTRAPOLATION) {
		// do extrapolation here
	}
}

bool started_updates = false;
void update(update_info_t& update_info, int transform_handle, snapshots_fifo_t& snapshot_fifo) {

	if (!started_updates && snapshot_fifo.get_size() == NUM_SNAPSHOTS_FOR_SAFE_INTERPOLATION) {
		started_updates = true;	
		assign_interpolating_snapshots(snapshot_fifo, update_info);
		assert(update_info.update_mode == update_mode_t::INTERPOLATION);
		platformer::time_t::cur_time = update_info.snapshot_from.game_time;
	}

	if (!started_updates) return;

	handle_snapshots(snapshot_fifo, update_info);
	update_object_positions(update_info, transform_handle);
	platformer::time_t::cur_time += platformer::time_t::delta_time;

	// basically if snapshot fifo is getting too packed, temporarily speed up time so that we don't have to drop snapshots
	// float multiplier = 1.0f;
	// bool need_speedup = snapshot_fifo.get_size() > (NUM_SNAPSHOTS_FOR_SAFE_INTERPOLATION * 1.5f);
	// if (need_speedup) {
	// 	int size = snapshot_fifo.get_size();
	// 	float INITIAL_SPEED_UP = 1.5f;
	// 	float MAX_SPEEDUP = 2.f;
	// 	float percentage_to_max_from_safe = remap(size, NUM_SNAPSHOTS_FOR_SAFE_INTERPOLATION * 1.5f, MAX_SNAPSHOT_BUFFER_SIZE, 0.f, (MAX_SPEEDUP - INITIAL_SPEED_UP));
	// 	multiplier = percentage_to_max_from_safe +  INITIAL_SPEED_UP;
	// }
	// platformer::time_t::cur_time += platformer::time_t::delta_time * multiplier;
}
