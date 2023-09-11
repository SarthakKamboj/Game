#pragma once

#include "shared/world/world.h"
#include "shared/utils/timer.h"
#include "shared/networking/networking.h"
#include "shared/utils/fifo.h"

#define NUM_SNAPSHOTS_FOR_SAFE_INTERPOLATION 3

typedef utils::fifo<world::snapshot_t, MAX_SNAPSHOT_BUFFER_SIZE> snapshots_fifo_t;

namespace world {
    enum class OBJECT_UPDATE_MODE {
        NONE = 0,
        INTERPOLATION,
        EXTRAPOLATION
    };

    enum class PLAYER_UPDATE_MODE {
        NONE = 0,
        INTERPOLATION,
        PREDICTION
    };

    struct obj_update_info_t {
        OBJECT_UPDATE_MODE update_mode = OBJECT_UPDATE_MODE::NONE;	
        OBJECT_UPDATE_MODE last_frame_update_mode = OBJECT_UPDATE_MODE::NONE;
        snapshot_t snapshot_from;
        snapshot_t* snapshot_to = NULL;

        float last_delta_x = 0.f;
        float last_delta_y = 0.f;
        time_count_t last_extrapolation_time = 0;
        float target_x;
        float target_y;
        // time_count_t cur_time = 0;
    };

    struct player_update_info_t {
        PLAYER_UPDATE_MODE update_mode = PLAYER_UPDATE_MODE::NONE;	
        snapshot_t snapshot_from;
        snapshot_t* snapshot_to = NULL;
        time_count_t cur_time = 0;
    };

    bool assign_interpolating_snapshots(obj_update_info_t& update_info);
    void handle_snapshots(obj_update_info_t& update_info);
    void update_interpolated_objs(obj_update_info_t& update_data);
    void update_player();
    void update(obj_update_info_t& update_data);
    void reset();

    void receive_snapshot(networking::server_cmd_t& server_cmd);
}