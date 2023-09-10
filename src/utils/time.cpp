#include "time.h"
#include "SDL.h"

time_count_t platformer::time_t::delta_time = 0;
time_count_t platformer::time_t::cur_time = 0;
time_count_t platformer::time_t::cur_independent_time = 0;


// time_count_t platformer::get_time_since_start_in_sec() {
// 	Uint32 ms_time = SDL_GetTicks();
// 	return ms_time / 1000.f;
// }
