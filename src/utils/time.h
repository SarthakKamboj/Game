#pragma once

#include "shared/utils/timer.h"

namespace platformer {
	struct time_t {
		static time_count_t delta_time;
		static time_count_t cur_time;
		static time_count_t cur_independent_time;
	};
	// time_count_t get_time_since_start_in_sec();
}
