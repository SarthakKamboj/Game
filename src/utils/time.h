#pragma once

#include "shared/utils/timer.h"

namespace platformer {
	struct time_t {
		static time_count_t delta_time;
		static time_count_t cur_time;
		static time_count_t cur_server_time;
	};
}
