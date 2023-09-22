#include "update.h"
#include "transform/transform.h"
#include "utils/time.h"
#include "utils/math.h"
#include "input/input.h"
#include "shared/input/input.h"
#include <cassert>

#include "test_config.h"

bool started_updates = false;

// extern int object_transform_handle; 
extern std::vector<int> interpolated_obj_transforms;
extern int player_transform_handle; 

unsigned int from_snapshot_id = INVALID_SNAPSHOT_ID;
unsigned int to_snapshot_id = INVALID_SNAPSHOT_ID;

#ifdef RUN_TESTCASES
snapshots_fifo_t snapshot_fifo;
#else
static snapshots_fifo_t snapshot_fifo;
#endif

namespace world {
    void reset() {
        from_snapshot_id = INVALID_SNAPSHOT_ID;
        to_snapshot_id = INVALID_SNAPSHOT_ID;
        while (!snapshot_fifo.empty) {
            snapshot_fifo.dequeue();
        }
    }

    void update_player_position(input::user_input_t input) {
        if (input.w_pressed) {
            transform_t* player_transform = get_transform(player_transform_handle);
            player_transform->position.x += 10;
        }
    }

    void resync_ack_user_cmd(gameobject_snapshot_cmd_t& go_cmd) {
        unsigned int user_cmd_response_id = go_cmd.input_cmd_response;
        input::user_cmd_t orig_user_cmd = input::clear_user_cmds_upto_id(user_cmd_response_id);
        
        transform_t* player_transform = get_transform(player_transform_handle);
        
        // set player to correct position
        player_transform->position.x = go_cmd.gameobject.x;
        player_transform->position.y = go_cmd.gameobject.y;

        // re-simulate registered user inputs ater this one has been confirmed

        // TODO: eventually limit when you send inputs to only something was acc pressed so that at some pt, if player is not moving,
        // game can sync in what the player position is supposed to be b/c of last_invalid_snapshot on gameobject_snapshot_t
        // this can help sync in player completely while it is idle and the current snapshot id > last_invalid_snapshot but no inputs needs to be processed/predicted upon
        // via interpolation (this could acc save a lot of processing power)
        // (can go away from n^2 algorithm to something less on the average)
        std::vector<input::user_cmd_t> cmds_to_replay = input::get_cmds_after_id(user_cmd_response_id);
        for (int i = 0; i < cmds_to_replay.size(); i++) {
            input::user_cmd_t& cmd_to_replay = cmds_to_replay[i];
            input::user_input_t& user_input = cmd_to_replay.user_input;
            update_player_position(user_input);
        }
    }

    void receive_snapshot(networking::server_cmd_t& server_cmd) {
        if (server_cmd.res_type == networking::SERVER_CMD_TYPE::SNAPSHOT) {
            static unsigned int last_enqueued_snapshot_id = 0;
            snapshot_t* snapshot = reinterpret_cast<snapshot_t*>(server_cmd.server_cmd_data);
            snapshot_fifo.enqueue(*snapshot);
        } else if (server_cmd.res_type == networking::SERVER_CMD_TYPE::USER_CMD_ACK) {
            gameobject_snapshot_cmd_t* go_snapshot = reinterpret_cast<gameobject_snapshot_cmd_t*>(server_cmd.server_cmd_data);
            resync_ack_user_cmd(*go_snapshot);
        }
    }

    // TODO: look at cur time assiging when the incoming snapshot after extrapolating is >= 2/3 frames than the last used frame
    bool assign_interpolating_snapshots(interpolated_obj_update_info_t& update_info) {
        snapshots_fifo_t::dequeue_state_t dequeue_state = snapshot_fifo.dequeue();
        if (dequeue_state.valid) {
            update_info.snapshot_from = dequeue_state.val;
        } else {
            // TODO: this is most likely that we have nothing to even interpolate from or to
            // LEAST IDEAL SITUATION TO BE IN
            return false;
        }

        snapshots_fifo_t::peek_state_t peek_state = snapshot_fifo.peek_read();
        if (peek_state.valid) {
            update_info.snapshot_to = peek_state.val;
        } else {
            snapshot_fifo.enqueue(update_info.snapshot_from);
            update_info.update_mode = OBJECT_UPDATE_MODE::EXTRAPOLATION;
            return false;
        }

        update_info.update_mode = OBJECT_UPDATE_MODE::INTERPOLATION;
        from_snapshot_id = dequeue_state.val.snapshot_id;
        to_snapshot_id = peek_state.val->snapshot_id;

        return true;
    }

    void handle_snapshots(interpolated_obj_update_info_t& update_info) {
        snapshot_t& snapshot_from = update_info.snapshot_from;
        snapshot_t*& snapshot_to = update_info.snapshot_to;	
        if (update_info.last_frame_update_mode == OBJECT_UPDATE_MODE::EXTRAPOLATION || platformer::time_t::cur_time >= snapshot_to->game_time) {
            assign_interpolating_snapshots(update_info);
        }
    }

    void update_interpolated_objs(interpolated_obj_update_info_t& update_data) {
        for (int i = 0; i < interpolated_obj_transforms.size(); i++) {
            int object_transform_handle = interpolated_obj_transforms[i];
            transform_t* transform_ptr = get_transform(object_transform_handle);
            assert(transform_ptr != NULL);

            const time_count_t extrap_fix_time = 0.7;

            if (update_data.update_mode == OBJECT_UPDATE_MODE::INTERPOLATION) {
                static math::smooth_damp_info_t x_damp_info;
                static math::smooth_damp_info_t y_damp_info;
                y_damp_info.total_time = extrap_fix_time;
                x_damp_info.total_time = extrap_fix_time;

                if (update_data.last_frame_update_mode == OBJECT_UPDATE_MODE::EXTRAPOLATION) {
                    x_damp_info.start_time = update_data.last_extrapolation_time;
                    y_damp_info.start_time = update_data.last_extrapolation_time;

                    x_damp_info.finished = false;
                    y_damp_info.finished = false;
                }

                snapshot_t& snapshot_from = update_data.snapshot_from;
                snapshot_t*& snapshot_to = update_data.snapshot_to;
                float iter_val = (platformer::time_t::cur_time - snapshot_from.game_time) / (snapshot_to->game_time - snapshot_from.game_time);
                float prev_x = transform_ptr->position.x;
                float prev_y = transform_ptr->position.y;

                float target_x = math::lerp(snapshot_from.gameobjects[i].x, snapshot_to->gameobjects[i].x, iter_val);
                float target_y = math::lerp(snapshot_from.gameobjects[i].y, snapshot_to->gameobjects[i].y, iter_val);


                // disabled smoothing for now since smoothing should probably be used mainly when there are
                // some errors that need to be fixed, such as after extrapolation (main one I can think of right now)
                // since as long as snapshots are coming in, we can just use snapshots to determine positions 

                // https://www.reddit.com/r/gamedev/comments/4zbrgp/how_does_unitys_smoothdamp_work/
                // TODO: read up on smooth damper functions and improve smoothing to be more seamless
                // but this is good for now

                static bool fixing_extrap_error = false;
                if (fixing_extrap_error || update_data.last_frame_update_mode == OBJECT_UPDATE_MODE::EXTRAPOLATION) {
                    fixing_extrap_error = true;
                    float smoothed_y = math::smooth_damp(prev_y, target_y, y_damp_info);
                    float smoothed_x = math::smooth_damp(prev_x, target_x, x_damp_info);

                    transform_ptr->position.x = smoothed_x;
                    transform_ptr->position.y = smoothed_y;

                    if (x_damp_info.finished && y_damp_info.finished) {
                        fixing_extrap_error = false;
                    }
                }
                else {
                    transform_ptr->position.x = target_x;
                    transform_ptr->position.y = target_y;
                }

                if (update_data.last_frame_update_mode == OBJECT_UPDATE_MODE::INTERPOLATION) {
                    transform_ptr->last_delta_x = transform_ptr->position.x - prev_x;
                    transform_ptr->last_delta_y = transform_ptr->position.y - prev_y;
                }

            }
            else if (update_data.update_mode == OBJECT_UPDATE_MODE::EXTRAPOLATION) {
                transform_ptr->position.x += transform_ptr->last_delta_x;
                transform_ptr->position.y += transform_ptr->last_delta_y;
                update_data.last_extrapolation_time = platformer::time_t::cur_time;
            }

            update_data.last_frame_update_mode = update_data.update_mode;
        }
    }

    void update_player() {

    }

    void update(interpolated_obj_update_info_t& update_info) {
        if (!started_updates && snapshot_fifo.get_size() == NUM_SNAPSHOTS_FOR_SAFE_INTERPOLATION) {
            started_updates = true;	
            assign_interpolating_snapshots(update_info);
            assert(update_info.update_mode == OBJECT_UPDATE_MODE::INTERPOLATION);
            platformer::time_t::cur_time = update_info.snapshot_from.game_time;
        }

        if (!started_updates) return;

        handle_snapshots(update_info);
        update_interpolated_objs(update_info);
        update_player();
    }
}