#include "update.h"
#include "transform/transform.h"
#include "utils/time.h"
#include "utils/math.h"
#include "input/input.h"
#include <cassert>
#include "physics/physics.h"

#include "test_config.h"

extern input::user_input_t input_state;
// extern int object_transform_handle;

namespace world {
    

    void update(application_t& app) {
        // update_player(app.main_character);
        app.main_character.update(input_state);
        // app.camera.update(input_state);
        update_rigidbodies();
    }
}