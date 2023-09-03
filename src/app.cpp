#include "app.h" 
#include "transform/transform.h"
#include "utils/time.h"
#include "constants.h"

bool set_interpolating_snapshots(fifo<TYPE_OF_MEMBER(res_data_t, snapshot_data), MAX_SNAPSHOT_BUFFER_SIZE>& snapshot_fifo, snapshot_data_t& snapshot_from, snapshot_data_t*& snapshot_to) {
	fifo<TYPE_OF_MEMBER(res_data_t, snapshot_data), MAX_SNAPSHOT_BUFFER_SIZE>::dequeue_state_t dequeue_state = snapshot_fifo.dequeue();
	if (dequeue_state.valid) {
		snapshot_from = dequeue_state.val;
		// std::cout << "dequeue valid and got snapshot " << snapshot_from.snapshot_id << std::endl;
	} else {
		// TODO: this is most likely that we have nothing to even interpolate from or to
		// LEAST IDEAL SITUATION TO BE IN
		std::cout << "dequeue was not valid" << std::endl;
		return false;
	}

	fifo<TYPE_OF_MEMBER(res_data_t, snapshot_data), MAX_SNAPSHOT_BUFFER_SIZE>::peek_state_t peek_state = snapshot_fifo.peek_read();
	if (peek_state.valid) {
		snapshot_to = peek_state.val;
	}	 else {
		// TODO: this is most likely situation where we do not have anything to interpolate to so we will need to do extrapolation until the next snapshot comes
		std::cout << "peek was not valid" << std::endl;
		// put the from snapshot back in to be used later
		snapshot_fifo.enqueue(snapshot_from);
		return false;
	}

	return true;
}

float remap(float val, float orig_low, float orig_high, float new_low, float new_high) {
	return new_low + (val - orig_low) * ((new_high - new_low) / (orig_high - orig_low));
}

bool started_updates = false;
void update(int transform_handle, fifo<TYPE_OF_MEMBER(res_data_t, snapshot_data), MAX_SNAPSHOT_BUFFER_SIZE>& snapshot_fifo) {
	static snapshot_data_t snapshot_from;
	static snapshot_data_t* snapshot_to;
	static float cur_time = 0.f;

	if (!started_updates && snapshot_fifo.get_size() == NUM_SNAPSHOTS_FOR_SAFE_INTERPOLATION) {
		started_updates = true;	
		set_interpolating_snapshots(snapshot_fifo, snapshot_from, snapshot_to);
		cur_time = snapshot_from.game_time;
		// std::cout << snapshot_from.game_time << " " << snapshot_to->game_time << std::endl;
		// std::cout << cur_time << std::endl;
	}

	if (!started_updates) return;

	
	// fifo<TYPE_OF_MEMBER(res_data_t, snapshot_data), MAX_SNAPSHOT_BUFFER_SIZE>::dequeue_state_t dequeue_state = snapshot_fifo.dequeue();
	// if (dequeue_state.valid) {
	// 	snapshot_from = dequeue_state.val;
	// 	cur_time = snapshot_from.game_time;
	// }

	// fifo<TYPE_OF_MEMBER(res_data_t, snapshot_data), MAX_SNAPSHOT_BUFFER_SIZE>::peek_state_t peek_state = snapshot_fifo.peek_read();
	// if (peek_state.valid) {
	// 	snapshot_to = peek_state.val;
	// }

	float time_from_older_snapshot = cur_time - snapshot_from.game_time;
	float time_between_snapshots = snapshot_to->game_time - snapshot_from.game_time;
	float iter_val = time_from_older_snapshot / time_between_snapshots;
	// std::cout << "iter_val: " << iter_val << std::endl;
	static float prev_iter_val = 0.f;

	if (iter_val >= 1.0f) {
		bool res = set_interpolating_snapshots(snapshot_fifo, snapshot_from, snapshot_to); 

		if (!res) {
			cur_time += platformer::time_t::delta_time;	
			std::cout << "extrapolating here" << std::endl;
			return;
		}

		iter_val = (cur_time - snapshot_from.game_time) / (snapshot_to->game_time - snapshot_from.game_time);
		if (std::isinf(iter_val)) {
			std::cout << "iter_val is inf: " << iter_val << std::endl;
		}
	}

	transform_t* transform_ptr = get_transform(transform_handle);
	assert(transform_ptr != NULL);

	transform_ptr->position.x = iter_val * (snapshot_to->gameobject_snapshots[0].x - snapshot_from.gameobject_snapshots[0].x) + snapshot_from.gameobject_snapshots[0].x;
	transform_ptr->position.y = iter_val * (snapshot_to->gameobject_snapshots[0].y - snapshot_from.gameobject_snapshots[0].y) + snapshot_from.gameobject_snapshots[0].y;

	float multiplier = 1.0f;

	// basically if snapshot fifo is getting too packed, temporarily speed up time so that we don't have to drop snapshots
	bool need_speedup = snapshot_fifo.get_size() > (NUM_SNAPSHOTS_FOR_SAFE_INTERPOLATION * 1.5f);
	if (need_speedup) {
		int size = snapshot_fifo.get_size();
		float INITIAL_SPEED_UP = 1.5f;
		float MAX_SPEEDUP = 2.f;
		float percentage_to_max_from_safe = remap(size, NUM_SNAPSHOTS_FOR_SAFE_INTERPOLATION * 1.5f, MAX_SNAPSHOT_BUFFER_SIZE, 0.f, (MAX_SPEEDUP - INITIAL_SPEED_UP));
		multiplier = percentage_to_max_from_safe +  INITIAL_SPEED_UP;
	}

	std::cout << "snapshot size: " << snapshot_fifo.get_size() << std::endl;
	// std::cout << "multiplier: " << multiplier << std::endl;

	cur_time += platformer::time_t::delta_time * multiplier;
	prev_iter_val = iter_val; 
	// cur_time += platformer::time_t::delta_time;
	// std::cout << "cur_time: " << cur_time << std::endl;
	
}
