#pragma once

struct animation_t {
    int handle = -1;
    // start and end values of the float the animation is animating
	float start = 0.f, end = 0.f;
    // pointer to the value the animation is animating
	float* cur = nullptr;
	float runtime = 0.f;
    float time_elapsed = 0.f;
    float start_time = 0.f;
    // whether animation is active or not
	bool enabled = false;
};

int create_animation(float* value, float start_val, float end_val, float runtime);
animation_t* get_animation(int anim_handle);
void update_animations();
void start_animation(int anim_handle);
