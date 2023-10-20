#pragma once

#if 1

#include "app.h"

namespace world {
    /// <summary>
    /// update the player object (maybe moved into its own file at some point)
    /// </summary>
    void update_player(main_character_t& main_character);

    /// <summary>
    /// updates the game. right now it updates the player and the rigidbodies in the game
    /// </summary>
    void update(application_t& app);
}

#else

#include "shared/world/world.h"
#include "shared/utils/timer.h"
#include "shared/networking/networking.h"
#include "shared/utils/fifo.h"

#define num_snapshots_for_safe_interpolation 3

typedef utils::fifo<world::snapshot_t, max_snapshot_buffer_size> snapshots_fifo_t;

namespace world {
    enum class object_update_mode {
        none = 0,
        interpolation,
        extrapolation
    };

    enum class player_update_mode {
        none = 0,
        interpolation,
        prediction
    };

    struct interpolated_obj_update_info_t {
        snapshot_t snapshot_from;
        time_count_t last_extrapolation_time = 0;
        object_update_mode update_mode = object_update_mode::none;	
        object_update_mode last_frame_update_mode = object_update_mode::none;
        snapshot_t* snapshot_to = null;
    };

    struct player_update_info_t {
        player_update_mode update_mode = player_update_mode::none;	
        snapshot_t snapshot_from;
        snapshot_t* snapshot_to = null;
        time_count_t cur_time = 0;
    };

    bool assign_interpolating_snapshots(interpolated_obj_update_info_t& update_info);
    void handle_snapshots(interpolated_obj_update_info_t& update_info);
    void update_interpolated_objs(interpolated_obj_update_info_t& update_data);

    /// <summary>
    /// update the player object (maybe moved into its own file at some point)
    /// </summary>
    void update_player();
    void update(interpolated_obj_update_info_t& update_data);
    void reset();

    void receive_snapshot(networking::server_cmd_t& server_cmd);

    /// <summary>
    /// updates the game. right now it updates the player and the rigidbodies in the game
    /// </summary>
    void update();
}
#endif