#pragma once

#include "time.h"

namespace math {
    struct smooth_damp_info_t {
        time_count_t total_time = 0;
        time_count_t start_time = 0;
        bool finished = true;
    };

    float lerp(float start, float end, float ratio);
    float remap(float val, float orig_low, float orig_high, float new_low, float new_high);
    float clamp(float val, float low, float high);

    // need to find a way to replicate this smooth damping on the server for acknowledgement and verification
    float smooth_damp(float current, float target, smooth_damp_info_t& damp_info);
    float smooth_damp(float current, float target, float speed, bool& finished);
}