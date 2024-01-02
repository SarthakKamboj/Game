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
// extern int object_transform_handle;

namespace world {
    

    void update(application_t& app) {

        if (app.scene_manager.cur_level == MAIN_MENU_LEVEL) {
            if (input_state.some_key_pressed) {
                app.scene_manager.queue_level_load = true;
                app.scene_manager.level_to_load = 1;
            }
            return;
        }
        if (app.scene_manager.cur_level == GAME_OVER_SCREEN_LEVEL) return;

        update_parallax_bcks(app.camera);

        update_rigidbodies();

        update_image_anim_players();

        // update_player(app.main_character);
        app.main_character.update(app, input_state);
        gos_update();
        app.camera.update(input_state, app.main_character);
    }
}