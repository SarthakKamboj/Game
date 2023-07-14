#include "animation.h"
#include "utils/time.h"
#include <vector>
#include <iostream>
#include <cassert>

std::vector<animation_t> animations;

void update_animations() {
	for (animation_t& animation : animations) {
		if (animation.enabled) {
			animation.time_elapsed = platformer::get_time_since_start_in_sec() - animation.start_time;
			animation.enabled = animation.time_elapsed <= animation.runtime;
			if (animation.enabled) {
				*animation.cur = (animation.time_elapsed / animation.runtime) * (animation.end - animation.start) + animation.start;
			}
		}
	}
}

animation_t* get_animation(int anim_handle) {
    for (int i = 0; i < animations.size(); i++) {
        if (anim_handle == animations[i].handle) {
            return &animations[i];
        }
    }
    return NULL;
}

void start_animation(int anim_handle) {
    animation_t* anim_ptr = get_animation(anim_handle);
    assert(anim_ptr != NULL);
	animation_t& animation = *anim_ptr;
	animation.time_elapsed = 0.f;
	animation.enabled = true;
	animation.start_time = platformer::get_time_since_start_in_sec();
}

int create_animation(float* value, float start_val, float end_val, float runtime) {
    static int running_cnt = 0;
    assert(value != NULL);
	animation_t animation;
    animation.handle = running_cnt;
	animation.cur = value;
	animation.start = start_val;
	animation.end = end_val;
	animation.enabled = false;
	animation.runtime = runtime;
    running_cnt++;
	animations.push_back(animation);
	return animation.handle;
}