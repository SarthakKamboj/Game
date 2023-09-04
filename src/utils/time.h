#pragma once

namespace platformer {
	struct time_t {
		static float delta_time;
		static float cur_time;
		static float cur_independent_time;
	};

	float get_time_since_start_in_sec();
}
