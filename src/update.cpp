#include "update.h"
#include "transform/transform.h"
#include "utils/time.h"
#include "input/input.h"
#include "shared/input/input.h"
#include <cassert>

#define SMOOTHING_ENABLED 0

bool started_updates = false;

extern int object_transform_handle; 
extern int player_transform_handle; 

typedef utils::fifo<world::snapshot_t, MAX_SNAPSHOT_BUFFER_SIZE> snapshots_fifo_t;

unsigned int from_snapshot_id = INVALID_SNAPSHOT_ID;
unsigned int to_snapshot_id = INVALID_SNAPSHOT_ID;

namespace world {

    static snapshots_fifo_t snapshot_fifo;

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
        // if (!user_cmd_peek.valid) return;
        // if (user_cmd_response_id < user_cmd_peek.val->input_cmd_id) return;
        // while (user_cmd_peek.valid && user_cmd_peek.val->input_cmd_id != user_cmd_response_id) {
        // 	user_cmds_fifo.dequeue();
        // 	user_cmd_peek = user_cmds_fifo.peek_read();
        // }
        // if (!user_cmd_peek.valid) return;
        // user_cmds_fifo_t::dequeue_state_t dequeud = user_cmds_fifo.dequeue();
        // user_cmd_t& orig_user_cmd = dequeud.val;
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

        // for (int i = 0; i < user_cmds_fifo.get_size(); i++) {
        // 	user_cmds_fifo_t::peek_state_t peek = user_cmds_fifo.get_at_idx(i);
        // 	if (peek.valid) {
        // 		user_cmd_t& user_cmd = *peek.val;
        // 		user_input_t& user_input = user_cmd.user_input;
        // 		input_state_t input_state;
        // 		input_state.key_state.key_down['w'] = user_input.w_pressed;
        // 		input_state.key_state.key_down['a'] = user_input.a_pressed;
        // 		input_state.key_state.key_down['s'] = user_input.s_pressed;
        // 		input_state.key_state.key_down['d'] = user_input.d_pressed;
        // 		update_player_position(input_state, player_handle);
        // 	}
        // }
    }

    void receive_snapshot(networking::server_cmd_t& server_cmd) {
        if (server_cmd.res_type == networking::SERVER_CMD_TYPE::SNAPSHOT) {
            static unsigned int last_enqueued_snapshot_id = 0;
            snapshot_t* snapshot = reinterpret_cast<snapshot_t*>(server_cmd.server_cmd_data);
            // snapshot_t& snapshot = server_res.res_data.snapshot_data;
            // snapshots_fifo_t& snapshot_fifo = network_info.snapshots_fifo;
            
            // snapshots_fifo_t::peek_state_t peek_state = snapshot_fifo.peek_read();
            // // snapshot receiving out of order logic
            // if (peek_state.valid && snapshot.snapshot_id < peek_state.val->snapshot_id) {
            //     std::cout << "cannot render this because it is too far back in interpolation time on client" << std::endl;
            //     return;
            // }
            // std::cout << "can hold " << snapshot_fifo.get_size() << " frames at a time" << std::endl;
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
            // TODO: this is most likely situation where we do not have anything to interpolate to so we will need to do extrapolation until the next snapshot comes
            // put the from snapshot back in to be used later
            snapshot_fifo.enqueue(update_info.snapshot_from);
            update_info.update_mode = OBJECT_UPDATE_MODE::EXTRAPOLATION;
            std::cout << "peek was not valid since snapshot is size " << snapshot_fifo.get_size() << std::endl;
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

    // need to find a way to replicate this smooth damping on the server for acknowledgement and verification
    float smooth_damp(float current, float target, float speed) {
        static float cur_vel = 0.0f;
        float diff = target - current;
        // float damp_factor = 1.f / smooth_time;
        float damp_factor = speed;
        cur_vel = damp_factor * diff * platformer::time_t::delta_time;
        float smooth_val = current + cur_vel * platformer::time_t::delta_time;
        if (abs((target - smooth_val) / diff) <= 0.02f) {
            smooth_val = target;
            cur_vel = 0.0f;
        }
        return smooth_val;
    }

    void update_interpolated_objs(obj_update_info_t& update_data) {
        transform_t* transform_ptr = get_transform(object_transform_handle);
        assert(transform_ptr != NULL);

        static float last_delta_x = 0.f;
        static float last_delta_y = 0.f;
        static time_count_t last_extrapolation_time = 0;
        static OBJECT_UPDATE_MODE last_mode = OBJECT_UPDATE_MODE::INTERPOLATION;

        if (update_data.update_mode == OBJECT_UPDATE_MODE::INTERPOLATION) {
            snapshot_t& snapshot_from = update_data.snapshot_from;
            snapshot_t*& snapshot_to = update_data.snapshot_to;
            float iter_val = (platformer::time_t::cur_time - snapshot_from.game_time) / (snapshot_to->game_time - snapshot_from.game_time);
            float prev_x = transform_ptr->position.x;
            float prev_y = transform_ptr->position.y;

            float target_x = lerp(snapshot_from.gameobjects[0].x, snapshot_to->gameobjects[0].x, iter_val);
            float target_y = lerp(snapshot_from.gameobjects[0].y, snapshot_to->gameobjects[0].y, iter_val);

#if SMOOTHING_ENABLED

            // https://www.reddit.com/r/gamedev/comments/4zbrgp/how_does_unitys_smoothdamp_work/
            // TODO: read up on smooth damper functions and improve smoothing to be more seamless
            // but this is good for now
            float smoothed_x = target_x;
            float smoothed_y = target_y;

#if 1
            const float NON_SMOOTH_THRESHOLD = 5.f;
			float speed = 10000;
            if (target_x - prev_x >= NON_SMOOTH_THRESHOLD)  {
                std::cout << "smoothing x" << std::endl;
				smoothed_x = smooth_damp(prev_x, target_x, speed);
            }
            
            static int i = 0;
            i++;

            if (i == 100) {
                std::cout << "hi" << std::endl;
            }

            if (target_y - prev_y >= NON_SMOOTH_THRESHOLD) {
                std::cout << "smoothing y" << std::endl;
                smoothed_y = smooth_damp(prev_y, target_y, speed);
            }
#else
            float speed = 5000;
            smoothed_x = smooth_damp(prev_x, target_x, speed);
            smoothed_y = smooth_damp(prev_y, target_y, speed);

#endif

            transform_ptr->position.x = smoothed_x;
            transform_ptr->position.y = smoothed_y;
#else
            transform_ptr->position.x = target_x;
            transform_ptr->position.y = target_y;
#endif

            if (last_mode == OBJECT_UPDATE_MODE::INTERPOLATION) {
                last_delta_x = transform_ptr->position.x - prev_x;
                last_delta_y = transform_ptr->position.y - prev_y;
            }

        } else if (update_data.update_mode == OBJECT_UPDATE_MODE::EXTRAPOLATION) {
            // do extrapolation here
            transform_ptr->position.x += last_delta_x;
            transform_ptr->position.y += last_delta_y;
            last_extrapolation_time = platformer::time_t::cur_time;

            std::cout << "extrapolating" << std::endl;

        }

        last_mode = update_data.update_mode;
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