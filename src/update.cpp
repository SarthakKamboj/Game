#include "update.h"
#include "transform/transform.h"
#include "utils/time.h"
#include "input/input.h"
#include "shared/input/input.h"
#include <cassert>

#include "test_config.h"

#define SMOOTHING_ENABLED 1

bool started_updates = false;

extern int object_transform_handle; 
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

    bool assign_interpolating_snapshots(obj_update_info_t& update_info) {
        snapshots_fifo_t::dequeue_state_t dequeue_state = snapshot_fifo.dequeue();
        if (dequeue_state.valid) {
            update_info.snapshot_from = dequeue_state.val;
        } else {
            // TODO: this is most likely that we have nothing to even interpolate from or to
            // LEAST IDEAL SITUATION TO BE IN
            // std::cout << "dequeue was not valid" << std::endl;
            return false;
        }

        snapshots_fifo_t::peek_state_t peek_state = snapshot_fifo.peek_read();
        if (peek_state.valid) {
            update_info.snapshot_to = peek_state.val;
        }	 else {
            snapshot_fifo.enqueue(update_info.snapshot_from);
            update_info.update_mode = OBJECT_UPDATE_MODE::EXTRAPOLATION;
            // std::cout << "peek was not valid since snapshot is size " << snapshot_fifo.get_size() << std::endl;
            return false;
        }

        update_info.update_mode = OBJECT_UPDATE_MODE::INTERPOLATION;
        from_snapshot_id = dequeue_state.val.snapshot_id;
        to_snapshot_id = peek_state.val->snapshot_id;

        // std::cout << "changed so that from_snapshot_id: " << from_snapshot_id << " and to_snapshot_id: " << to_snapshot_id << " with " << snapshot_fifo.get_size()-1 << " transitions left" << std::endl;

        return true;
    }

    void handle_snapshots(obj_update_info_t& update_info) {
        snapshot_t& snapshot_from = update_info.snapshot_from;
        snapshot_t*& snapshot_to = update_info.snapshot_to;	

        if (update_info.update_mode == OBJECT_UPDATE_MODE::EXTRAPOLATION || platformer::time_t::cur_time >= snapshot_to->game_time) {
            assign_interpolating_snapshots(update_info);
        }
    }

    float lerp(float start, float end, float ratio) {
        return ((end - start) * ratio) + start;
    }

    float remap(float val, float orig_low, float orig_high, float new_low, float new_high) {
        return new_low + (val - orig_low) * ((new_high - new_low) / (orig_high - orig_low));
    }

    float clamp(float val, float low, float high) {
        if (val < low) return low;
        if (val > high) return high;
        return val;
    }

    // need to find a way to replicate this smooth damping on the server for acknowledgement and verification
    float smooth_damp(float current, float target, smooth_damp_info_t& damp_info) {
#if 0
        if (damp_info.finished) return target;
        static const float SMOOTH_CONST = sqrt(0.0396f);
        float diff = target - current;
        if (diff == 0) {
            damp_info.finished = true;
            return target;
        }
        time_count_t time_elaspsed = platformer::time_t::cur_time - damp_info.start_time;
        float ratio = time_elaspsed / damp_info.total_time;
        float pt_98_multiplier = abs(0.98f * diff / SMOOTH_CONST);
        float multiplier = 2 * ratio * pt_98_multiplier;
        float final_multiplier = multiplier - pt_98_multiplier;
        float intermediate_diff = ((diff * final_multiplier) / sqrt((diff * diff) + (final_multiplier * final_multiplier)));
        float smooth_diff = (intermediate_diff + diff) * 0.5f;
        float smooth_val = smooth_diff + current;
        damp_info.finished = false;
        if (abs((target - smooth_val) / target) <= 0.02) {
            // std::cout << "finished cause of dist" << std::endl;
            smooth_val = target;
            damp_info.finished = true;
        } else if (abs((damp_info.total_time - time_elaspsed) / damp_info.total_time) <= 0.01) {
            // std::cout << "finished cause of time" << std::endl;
            smooth_val = target;
            damp_info.finished = true;
        }

        return smooth_val;
#else

        if (damp_info.finished) return target;
        static const float SMOOTH_CONST = sqrt(0.0396f);
        float diff = target - current;
        if (diff == 0) {
            damp_info.finished = true;
            return target;
        }
        time_count_t time_elaspsed = platformer::time_t::cur_time - damp_info.start_time;
        const int power = 1;
        float ratio = pow(time_elaspsed / damp_info.total_time, power);

#if 1
        float pt_98_multiplier = abs(0.98f * diff / SMOOTH_CONST);
        float multiplier = 2 * ratio * pt_98_multiplier;
        float final_multiplier = multiplier - pt_98_multiplier;
        float intermediate_diff = ((diff * final_multiplier) / sqrt((diff * diff) + (final_multiplier * final_multiplier)));
        float smooth_diff = (intermediate_diff + diff) * 0.5f;
        float smooth_val = smooth_diff + current;
#else
        float smooth_val = (diff * ratio) + current;
#endif
        damp_info.finished = false;
        if (abs(target - smooth_val) <= 0.01 || ratio >= 1.f) {
            smooth_val = target;
            damp_info.finished = true;
        }

        return smooth_val;
#endif
    }

    float smooth_damp(float current, float target, float speed, bool& finished) {
        static float cur_vel = 0.0f;
        float diff = target - current;
        float damp_factor = speed;
        cur_vel = damp_factor * diff * platformer::time_t::delta_time;
        float smooth_val = current + (cur_vel * platformer::time_t::delta_time);
        if (abs(target - smooth_val) > abs(target - current)) {
            std::cout << "here" << std::endl;
        }
        finished = false;
        if (abs(target - smooth_val) <= 0.05f) {
            smooth_val = target;
            cur_vel = 0.0f;
            finished = true;
        }
        return smooth_val;
    }

    void update_interpolated_objs(obj_update_info_t& update_data) {
        transform_t* transform_ptr = get_transform(object_transform_handle);
        assert(transform_ptr != NULL);
        
        const time_count_t extrap_fix_time = 0.7;

        if (update_data.update_mode == OBJECT_UPDATE_MODE::INTERPOLATION) {

            static smooth_damp_info_t x_damp_info;
            static smooth_damp_info_t y_damp_info;
            y_damp_info.total_time = extrap_fix_time;
            x_damp_info.total_time = extrap_fix_time;

            if (update_data.last_frame_update_mode == OBJECT_UPDATE_MODE::EXTRAPOLATION) {
                std::cout << "done extrapolating" << std::endl;
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

            float target_x = lerp(snapshot_from.gameobjects[0].x, snapshot_to->gameobjects[0].x, iter_val);
            float target_y = lerp(snapshot_from.gameobjects[0].y, snapshot_to->gameobjects[0].y, iter_val);
            
            update_data.target_x = target_x;
            update_data.target_y = target_y;

            // disabled smoothing for now since smoothing should probably be used mainly when there are
            // some errors that need to be fixed, such as after extrapolation (main one I can think of right now)
            // since as long as snapshots are coming in, we can just use snapshots to determine positions 

            // https://www.reddit.com/r/gamedev/comments/4zbrgp/how_does_unitys_smoothdamp_work/
            // TODO: read up on smooth damper functions and improve smoothing to be more seamless
            // but this is good for now

            static bool fixing_extrap_error = false;
            if (fixing_extrap_error || update_data.last_frame_update_mode == OBJECT_UPDATE_MODE::EXTRAPOLATION) {
                fixing_extrap_error = true; 
                float smoothed_y = smooth_damp(prev_y, target_y, y_damp_info);
                float smoothed_x = smooth_damp(prev_x, target_x, x_damp_info);

                transform_ptr->position.x = smoothed_x;
                transform_ptr->position.y = smoothed_y;

                if (x_damp_info.finished && y_damp_info.finished) {
                    fixing_extrap_error = false;
                }

            } else {
                transform_ptr->position.x = target_x;
                transform_ptr->position.y = target_y;
            }

            if (update_data.last_frame_update_mode == OBJECT_UPDATE_MODE::INTERPOLATION) {
                update_data.last_delta_x = transform_ptr->position.x - prev_x;
                update_data.last_delta_y = transform_ptr->position.y - prev_y;
            }

        } else if (update_data.update_mode == OBJECT_UPDATE_MODE::EXTRAPOLATION) {
            // do extrapolation here
            transform_ptr->position.x += update_data.last_delta_x;
            transform_ptr->position.y += update_data.last_delta_y;
            update_data.last_extrapolation_time = platformer::time_t::cur_time;

            if (update_data.last_frame_update_mode == OBJECT_UPDATE_MODE::INTERPOLATION) {
                std::cout << "started extrapolating" << std::endl;
            }

        }

        update_data.last_frame_update_mode = update_data.update_mode;
        // std::cout << "x: " << transform_ptr->position.x << " y: " << transform_ptr->position.y << std::endl;
    }

    void update_player() {

    }

    void update(obj_update_info_t& update_info) {
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