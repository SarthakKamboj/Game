#include "math.h"
#include "utils/time.h"
#include <iostream>

namespace math {
    float lerp(float start, float end, float ratio) {
        return ((end - start) * ratio) + start;
    }

    float remap(float val, float orig_low, float orig_high, float new_low, float new_high) {
        return new_low + (val - orig_low) * ((new_high - new_low) / (orig_high - orig_low));
    }

    float clamp(float val, float low, float high) {
        if (val < low) return low;
        if (val > high) return high;
        return val;
    }

    // need to find a way to replicate this smooth damping on the server for acknowledgement and verification
    float smooth_damp(float current, float target, smooth_damp_info_t& damp_info) {
    #if 0
        if (damp_info.finished) return target;
        static const float SMOOTH_CONST = sqrt(0.0396f);
        float diff = target - current;
        if (diff == 0) {
            damp_info.finished = true;
            return target;
        }
        time_count_t time_elaspsed = platformer::time_t::cur_time - damp_info.start_time;
        float ratio = time_elaspsed / damp_info.total_time;
        float pt_98_multiplier = abs(0.98f * diff / SMOOTH_CONST);
        float multiplier = 2 * ratio * pt_98_multiplier;
        float final_multiplier = multiplier - pt_98_multiplier;
        float intermediate_diff = ((diff * final_multiplier) / sqrt((diff * diff) + (final_multiplier * final_multiplier)));
        float smooth_diff = (intermediate_diff + diff) * 0.5f;
        float smooth_val = smooth_diff + current;
        damp_info.finished = false;
        if (abs((target - smooth_val) / target) <= 0.02) {
            // std::cout << "finished cause of dist" << std::endl;
            smooth_val = target;
            damp_info.finished = true;
        } else if (abs((damp_info.total_time - time_elaspsed) / damp_info.total_time) <= 0.01) {
            // std::cout << "finished cause of time" << std::endl;
            smooth_val = target;
            damp_info.finished = true;
        }

        return smooth_val;
    #else

        if (damp_info.finished) return target;
        static const float SMOOTH_CONST = sqrt(0.0396f);
        float diff = target - current;
        if (diff == 0) {
            damp_info.finished = true;
            return target;
        }
        time_count_t time_elaspsed = platformer::time_t::cur_time - damp_info.start_time;
        const int power = 1;
        float ratio = pow(time_elaspsed / damp_info.total_time, power);

    #if 1
        float pt_98_multiplier = abs(0.98f * diff / SMOOTH_CONST);
        float multiplier = 2 * ratio * pt_98_multiplier;
        float final_multiplier = multiplier - pt_98_multiplier;
        float intermediate_diff = ((diff * final_multiplier) / sqrt((diff * diff) + (final_multiplier * final_multiplier)));
        float smooth_diff = (intermediate_diff + diff) * 0.5f;
        float smooth_val = smooth_diff + current;
    #else
        float smooth_val = (diff * ratio) + current;
    #endif
        damp_info.finished = false;
        if (abs(target - smooth_val) <= 0.01 || ratio >= 1.f) {
            smooth_val = target;
            damp_info.finished = true;
        }

        return smooth_val;
    #endif
    }

    float smooth_damp(float current, float target, float speed, bool& finished) {
        static float cur_vel = 0.0f;
        float diff = target - current;
        float damp_factor = speed;
        cur_vel = damp_factor * diff * platformer::time_t::delta_time;
        float smooth_val = current + (cur_vel * platformer::time_t::delta_time);
        if (abs(target - smooth_val) > abs(target - current)) {
            std::cout << "here" << std::endl;
        }
        finished = false;
        if (abs(target - smooth_val) <= 0.05f) {
            smooth_val = target;
            cur_vel = 0.0f;
            finished = true;
        }
        return smooth_val;
    }
}