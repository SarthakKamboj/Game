#include "app.h" 
#include "gameobjects/gos.h"
#include "physics/physics.h"
#include "utils/time.h"
#include "animation/animation.h"

void update(key_state_t& key_state, const main_character_t& mc) {
	update_rigidbodies();
	update_animations();
	update_main_character(mc, key_state);
}
