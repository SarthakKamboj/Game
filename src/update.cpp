#include "update.h"
#include "transform/transform.h"
#include "utils/time.h"
#include "utils/math.h"
#include "input/input.h"
#include <cassert>
#include "physics/physics.h"

#include "test_config.h"

extern input::user_input_t input_state;
extern int object_transform_handle;

namespace world {
    void update_player() {
        glm::vec2 delta(0);
        if (input_state.d_down) {
            delta.x = 1;
        }
        else if (input_state.a_down) {
            delta.x = -1;
        }

        if (input_state.w_down) {
            delta.y = 1;
        }
        else if (input_state.s_down) {
            delta.y = -1;
        }

        if (delta.x != 0 || delta.y != 0) {
            const int speed = 200;
            delta = glm::normalize(delta) * static_cast<float>(platformer::time_t::delta_time * speed);
            // delta = glm::normalize(delta) * 2.f;
        }
        transform_t* player_transform = get_transform(object_transform_handle);
        assert(player_transform);
        player_transform->position += glm::vec3(delta, 0);
    }

    void update() {
        update_player();
        update_rigidbodies();
    }
}