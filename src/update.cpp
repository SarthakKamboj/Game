#include "update.h"
#include "transform/transform.h"
#include "utils/time.h"
#include "utils/math.h"
#include "input/input.h"
#include <cassert>
#include "physics/physics.h"
#include "gameobjects/gos.h"
#include "animation/animation.h"
#include <iostream>
#include "constants.h"

#include "test_config.h"

extern input::user_input_t input_state;

namespace world {
    
    void update(application_t& app) {

        if (app.scene_manager.cur_level <= MAIN_MENU_LEVEL || app.scene_manager.cur_level >= GAME_OVER_SCREEN_LEVEL) return;
        if (app.paused) return;

        app.time_spent_in_levels += platformer::time_t::delta_time;

        update_parallax_bcks(app.camera);

        update_rigidbodies();

        update_image_anim_players();

        // update_player(app.main_character);
        app.main_character.update(app, input_state);
        gos_update();
        app.camera.update(input_state, app.main_character);
    }
}