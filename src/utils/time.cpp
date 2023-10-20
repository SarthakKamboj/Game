#include "time.h"
#include "SDL.h"

time_count_t platformer::time_t::delta_time = 0;
time_count_t platformer::time_t::cur_time = 0;

namespace utils {
    void start_timer(game_timer_t& timer) {
        timer.running = true;
        auto start = std::chrono::high_resolution_clock::now();
        timer.start_time = std::chrono::duration_cast<game_time_t>(start.time_since_epoch());
    }

    void end_timer(game_timer_t& timer) {
        timer.running = false;
        auto end = std::chrono::high_resolution_clock::now();
        timer.end_time = std::chrono::duration_cast<game_time_t>(end.time_since_epoch());
        game_time_t elapsed = timer.end_time - timer.start_time;
        timer.elapsed_time_sec = elapsed.count();
    }
}
