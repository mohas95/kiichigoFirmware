#include "MotionCoordinator.h"
#include "Log.h"
#include <stdio.h>
#include <algorithm>
#include <cctype>
#include <cmath>

MotionCoordinator::MotionCoordinator(const MotionConfig& config, uint16_t log_rate) : log_rate_(log_rate) {
    
    for (StepperMotor* stepper : config.stepper_motors){
        stepper_motors_[stepper->label()] = stepper; 
    }

    for (LimitSwitch* limit_switch : config.limit_switches){
        limit_switches_[limit_switch->label()]=limit_switch;
    }

    register_commands_();

    last_log_time_us_ = time_us_64();

}

std::string MotionCoordinator::read_serial_line() {
    std::string buffer;
    int c;

    while (true) {
        c = getchar_timeout_us(0);  // non-blocking read

        if (c == PICO_ERROR_TIMEOUT) {
            break;  // no input, return what we have (or nothing)
        }

        if (c == '\r') continue;  // ignore carriage return

        if (c == '\n') {
            return buffer;  // full line received
        }

        buffer += static_cast<char>(c);
    }

    return "";  // nothing read
}


void MotionCoordinator::request_serial_action(){

    std::string line = read_serial_line();

    if (!line.empty()) {
        std::string command;
        std::istringstream iss(line);     
        iss >> command;

        std::transform(command.begin(), command.end(), command.begin(), ::toupper);

        if(command_handlers_.find(command) != command_handlers_.end()){
            
            command_handlers_[command](iss);

        }else{
            LOG_WARN("unknown command: %s\n", command.c_str());
        }

    }
    
}

//TODO: Limit switch integration, wait command
void MotionCoordinator::request_limit_switch_action() {

    for (const auto& [label, limit_switch] : limit_switches_) {

        bool triggered = limit_switch->get_state(true);

        if (!triggered) {
            // Reset only after the switch is released
            bool any_motor_active = false;

            for (const auto& motor_label : limit_switch->get_mapping()) {
                if (stepper_motors_.at(motor_label)->active()) {
                    any_motor_active = true;
                    break;
                }
            }

            if (!any_motor_active) {
                limit_switch->set_hit_flag(false);
            }

            continue;
        }

        if (limit_switch->get_hit_flag()) {
            continue;
        }

        std::ostringstream motors_to_stop;

        for (const auto& motor_label : limit_switch->get_mapping()) {

            auto* motor = stepper_motors_.at(motor_label);

            double reverse_revs = motor->active()
                ? limit_switch->get_reverse_value()
                : 0.0;

            motors_to_stop
                << motor_label << ","
                << limit_switch->get_fixed_position() << ","
                << reverse_revs << " ";
        }

        if (!motors_to_stop.str().empty()) {

            limit_switch->set_hit_flag(true);

            std::istringstream iss(motors_to_stop.str());

            command_handlers_.at("HIT")(iss);
        }
    }
}

// void MotionCoordinator::request_limit_switch_action(){

//     for (const auto&[label, limit_switch]: limit_switches_){
        
//         if(limit_switch->get_state(true)){
            
//             std::ostringstream motors_to_stop;
            
//             for(const std::string& motor_label : limit_switch->get_mapping()) {
//                 auto* motor = stepper_motors_.at(motor_label);
//                 // double rev_value = motor->active() ? limit_switch->get_reverse_value() : 0;

//                 if(!motor->active()){

//                     limit_switch->set_hit_flag(false);
//                     rev_value = 0;

//                     // LOG_INFO("INTERRUPT: HIT %s, pos: %0.2f, reversing: %0.2f revs\n", motor_label.c_str(), limit_switch->get_fixed_position(), rev_value);
//                 }else{

//                     if(limit_switch->get_hit_flag()){
//                         continue;
//                     }
//                     rev_value = limit_switch->get_reverse_value();
//                 }

//                 motors_to_stop << motor_label << "," << limit_switch->get_fixed_position() << "," << rev_value << " ";

//                 // if(std::find(disable_action_for_.begin(), disable_action_for_.end(), motor_label)==disable_action_for_.end()){ // only create command if it is not already hit
//                 //     double rev_value = with_reverse ? limit_switch->get_reverse_value() : 0;
//                 //     motors_to_stop << motor_label << "," << limit_switch->get_fixed_position() << "," << rev_value << " ";
//                 // }
//             }

//             if(!motors_to_stop.str().empty()){
//                 std::istringstream iss(motors_to_stop.str());
//                 command_handlers_["HIT"](iss);
//             }
//         }
//     }

// }

// void MotionCoordinator::clear_action_queue(const std::string& motor_label) {
//     if (motor_label.empty()) {
//         // Clear the entire action queue
//         std::queue<std::function<void()>> empty;
//         std::swap(action_queue_, empty);
//         LOG_INFO("Action queue cleared for all motors.\n");
//     } else {
//         // Clear actions related to a specific motor
//         std::queue<std::function<void()>> new_queue;
//         while (!action_queue_.empty()) {
//             auto action = action_queue_.front();
//             action_queue_.pop();

//             // Here you would need a way to check if the action is related to the specified motor.
//             // This is a placeholder for that logic.
//             bool is_related_to_motor = true; // Replace with actual check

//             if (!is_related_to_motor) {
//                 new_queue.push(action);
//             }
//         }
//         std::swap(action_queue_, new_queue);
//         LOG_INFO("Action queue cleared for motor: %s\n", motor_label.c_str());
//     }
// }

bool MotionCoordinator::action_active(){

    bool active_flag=false;

    for (const auto&[label, motor]: stepper_motors_){
        if(motor->active()){
            active_flag=true;
        }
    }
    return active_flag;
}


void MotionCoordinator::output_states() {

    uint64_t time_now = time_us_64();
    uint64_t time_diff = (time_now-last_log_time_us_)/1000; //in ms


    if (time_diff>=log_rate_){
        std::ostringstream output_state;

        for(const auto&[label, limit_switch]: limit_switches_){

            std::string switch_state = limit_switch->get_state(true) ? "activated": "off";

            output_state << label << "(limit_switch):" << switch_state << " ";

        }

        for (const auto&[label, motor]: stepper_motors_){

            if(!motor->get_standbyMode()){

                std::string dir = motor->get_direction() ? "+": "-";

                output_state << label << "(StepperMotor):" << motor->get_speed() << "(rpm)," 
                                                            <<  dir << ","
                                                            << motor->get_position_rev() << "(pos_rev),"
                                                            << motor->get_position_step() << "(pos_steps),"
                                                            << motor->get_position_deg() << "(pos_deg),"
                                                            << motor->get_position_deg_wrapped() << "(deg_wrapped),"
                                                            << " ";

            }else{
                output_state << label << "(StepperMotor):" << "Standby"<< " ";
            }


        }

        output_state_str_=output_state.str();
        
        LOG_OUTPUT("%s\n", output_state_str_.c_str());

        last_log_time_us_ = time_now;

    }

}



void MotionCoordinator::loop_forever(){
    while(true){
        request_serial_action();
        request_limit_switch_action();
        output_states();

        if(!action_active() && !action_queue_.empty()){
            auto action = std::move(action_queue_.front());
            action_queue_.pop();
            action();
            // while(action_active()){
            //     request_serial_action();
            // }
        }
        tight_loop_contents();
    }
}

std::vector<LimitSwitch*> MotionCoordinator::get_limit_switches(const std::string& mapping) const {
    std::vector<LimitSwitch*> switches;

    for (const auto& [label, limit_switch] : limit_switches_) {
        const auto& mappings = limit_switch->get_mapping();
        if (std::find(mappings.begin(), mappings.end(), mapping) != mappings.end()) {
            switches.push_back(limit_switch);
        }
    }

    return switches;
}



//private

void MotionCoordinator::register_commands_(){

    command_handlers_["MOVE"] = [&](std::istringstream& iss) {
        /* Command format:
        MOVE X,10.0,100.0 Y,100.0,100.0 Z,-100.0,100.0

        Each token: LABEL,REVOLUTIONS,SPEED_RPM
        This is a QUEUED command.
        */

        struct MoveCommand {
            double move_val;
            double speed;
        };

        std::streampos pos = iss.tellg();
        std::string full_line = (pos != std::streampos(-1))
            ? iss.str().substr(static_cast<std::size_t>(pos))
            : "";

        std::string token;
        std::unordered_map<std::string, MoveCommand> command_dict;

        while (iss >> token) {

            std::istringstream command_stream(token);
            std::string label, value_str, speed_str, extra;

            // Parse exactly three comma-separated fields
            if (!std::getline(command_stream, label, ',') ||
                !std::getline(command_stream, value_str, ',') ||
                !std::getline(command_stream, speed_str, ',') ||
                std::getline(command_stream, extra, ',')) {

                LOG_WARN("Invalid MOVE token: %s\n", token.c_str());
                continue;
            }

            // Validate motor
            if (stepper_motors_.find(label) == stepper_motors_.end()) {
                LOG_WARN("Unknown motor: %s\n", label.c_str());
                continue;
            }

            // Validate numeric values
            if (value_str.empty() || speed_str.empty() ||
                !is_float_(value_str) || !is_float_(speed_str)) {

                LOG_WARN("Invalid MOVE values: %s\n", token.c_str());
                continue;
            }

            double move_val = std::stod(value_str);
            double speed = std::stod(speed_str);

            if (!std::isfinite(move_val) ||
                !std::isfinite(speed) ||
                speed <= 0.0) {

                LOG_WARN("Invalid MOVE parameters: %s\n", token.c_str());
                continue;
            }

            command_dict[label] = {move_val, speed};
        }

        if (!command_dict.empty()) {

            action_queue_.push([command_dict, this]() {

                for (const auto& [label, cmd] : command_dict) {

                    stepper_motors_.at(label)->revolve(
                        cmd.move_val,
                        cmd.speed
                    );

                    LOG_INFO(
                        "Action: MOVE %s -> %0.2f revolutions at %0.2f RPM\n",
                        label.c_str(),
                        cmd.move_val,
                        cmd.speed
                    );
                }
            });

            LOG_INFO("Added to Queue: MOVE %s\n", full_line.c_str());

        } else {
            LOG_WARN(
                "No valid actions in MOVE command: %s\n",
                full_line.c_str()
            );
        }
    };

    command_handlers_["STOP"] = [&](std::istringstream& iss) {
        /* Command format:
        STOP X Y Z

        This is an IMMEDIATE command.
        */

        std::streampos pos = iss.tellg();
        std::string full_line = (pos != std::streampos(-1))
            ? iss.str().substr(static_cast<std::size_t>(pos))
            : "";

        std::string label;
        std::vector<std::string> command_vec;

        while (iss >> label) {

            if (stepper_motors_.find(label) == stepper_motors_.end()) {
                LOG_WARN("Unknown motor: %s\n", label.c_str());
                continue;
            }

            command_vec.push_back(label);
        }

        if (!command_vec.empty()) {

            // Clear pending actions
            std::queue<std::function<void()>> empty;
            action_queue_.swap(empty);

            // Stop specified motors immediately
            for (const auto& label : command_vec) {
                stepper_motors_.at(label)->stop();

                LOG_INFO("STOP motor: %s\n", label.c_str());
            }

            LOG_INFO("INTERRUPT: STOP %s\n", full_line.c_str());

        } else {
            LOG_WARN(
                "No valid motors in STOP command: %s\n",
                full_line.c_str()
            );
        }
    };
    
    command_handlers_["HIT"] = [&](std::istringstream& iss) { //test with limit switches
        /* Command format:
        HIT X,10.0,0.5 Y,200.5,1.0 Z,-30.0,2.0

        Each token: LABEL,POSITION,REVERSE_REVOLUTIONS
        This is an IMMEDIATE / INTERRUPT command.
        */

        struct HitCommand {
            double position;
            double reverse_revs;
            // bool previous_direction=true; 
        };

        std::streampos pos = iss.tellg();
        std::string full_line = (pos != std::streampos(-1))
            ? iss.str().substr(static_cast<std::size_t>(pos))
            : "";

        std::string token;
        std::unordered_map<std::string, HitCommand> command_dict;

        while (iss >> token) {

            std::istringstream command_stream(token);

            std::string label;
            std::string position_str;
            std::string reverse_str;
            std::string extra;

            // Parse exactly four comma-separated fields
            if (!std::getline(command_stream, label, ',') ||
                !std::getline(command_stream, position_str, ',') ||
                !std::getline(command_stream, reverse_str, ',') ||
                std::getline(command_stream, extra, ',')) {

                LOG_WARN("Invalid HIT token: %s\n", token.c_str());
                continue;
            }

            // Validate motor label
            if (stepper_motors_.find(label) == stepper_motors_.end()) {
                LOG_WARN("Unknown motor: %s\n", label.c_str());
                continue;
            }

            // Validate numeric inputs
            if (position_str.empty() || reverse_str.empty() ||
                !is_float_(position_str) ||
                !is_float_(reverse_str)) {

                LOG_WARN("Invalid HIT values: %s\n", token.c_str());
                continue;
            }

            double position = std::stod(position_str);
            double reverse_revs = std::stod(reverse_str);

            if (!std::isfinite(position) ||
                !std::isfinite(reverse_revs) ||
                reverse_revs < 0.0) {

                LOG_WARN("Invalid HIT parameters: %s\n", token.c_str());
                continue;
            }

            command_dict[label] = {
                position,
                reverse_revs
            };
        }

        if (!command_dict.empty()) {

            // Clear pending actions
            std::queue<std::function<void()>> empty;
            action_queue_.swap(empty);

            for (const auto& [label, command] : command_dict) {

                auto* motor = stepper_motors_.at(label);
                bool previous_direction = motor->get_direction();

                // Stop current movement before reversing
                motor->stop();

                // Update position at the hit location
                motor->set_position_rev(command.position);


                if (command.reverse_revs>0.0){
                    // Reverse relative to previous direction
                    double reverse_revs = previous_direction ? -command.reverse_revs : command.reverse_revs;
                    motor->revolve(reverse_revs, motor->get_default_speed());
                }

                LOG_INFO(
                    "INTERRUPT: HIT %s, pos: %0.2f, "
                    "reversing: %0.2f revs\n",
                    label.c_str(),
                    command.position,
                    command.reverse_revs
                );
            }

            // action_queue_.push([command_dict, this]() {

            //     for (const auto& [label, command] : command_dict) {
                    
            //         auto* motor = stepper_motors_.at(label);

                    
            //         // Reverse relative to previous direction
            //         double reverse_revs = command.previous_direction
            //             ? -command.reverse_revs
            //             : command.reverse_revs;

            //         motor->revolve(reverse_revs, motor->get_default_speed());


            //         // LOG_INFO(
            //         //     "Action: STANDBY %s -> %s\n",
            //         //     label.c_str(),
            //         //     value ? "enabled" : "disabled"
            //         // );
            //     }
            // });

            // action_queue_.push([command_dict, this]() {
            //     // Clear the hit flags after all actions are complete

            //     for (const auto& [label, value] : command_dict) {
            //         auto matching_switches = get_limit_switches(label);

            //         for (const auto& limit_switch : matching_switches) {
            //             limit_switch->set_hit_flag(false);
            //         }
            //     }
            // });

            LOG_INFO("INTERRUPT: HIT %s\n", full_line.c_str());

        } else {
            LOG_WARN(
                "No valid actions in HIT command: %s\n",
                full_line.c_str()
            );
        }
    };

    command_handlers_["STANDBY"] = [&](std::istringstream& iss) {
        /* Command format:
        STANDBY X,0 Y,1 Z,0

        Each token: MOTOR_LABEL,STANDBY_STATE
        0 = standby disabled
        1 = standby enabled

        This is a QUEUED command.
        */

        std::streampos pos = iss.tellg();
        std::string full_line = (pos != std::streampos(-1))
            ? iss.str().substr(static_cast<std::size_t>(pos))
            : "";

        std::string token;
        std::unordered_map<std::string, bool> command_dict;

        while (iss >> token) {

            std::istringstream command_stream(token);
            std::string label, value_str, extra;

            // Parse exactly two comma-separated fields
            if (!std::getline(command_stream, label, ',') ||
                !std::getline(command_stream, value_str, ',') ||
                std::getline(command_stream, extra, ',')) {

                LOG_WARN("Invalid STANDBY token: %s\n", token.c_str());
                continue;
            }

            // Validate motor label
            if (stepper_motors_.find(label) == stepper_motors_.end()) {
                LOG_WARN("Unknown motor: %s\n", label.c_str());
                continue;
            }

            // Validate boolean value
            if (value_str.empty() || !is_bool_(value_str)) {
                LOG_WARN("Invalid STANDBY value: %s\n", token.c_str());
                continue;
            }

            bool value = sto_bool_(value_str);

            command_dict[label] = value;
        }

        if (!command_dict.empty()) {

            action_queue_.push([command_dict, this]() {

                for (const auto& [label, value] : command_dict) {

                    stepper_motors_.at(label)->set_standbyMode(value);

                    LOG_INFO(
                        "Action: STANDBY %s -> %s\n",
                        label.c_str(),
                        value ? "enabled" : "disabled"
                    );
                }
            });

            LOG_INFO(
                "Added to Queue: STANDBY %s\n",
                full_line.c_str()
            );

        } else {
            LOG_WARN(
                "No valid actions in STANDBY command: %s\n",
                full_line.c_str()
            );
        }
    };


}


bool MotionCoordinator::is_integer_(const std::string& s) {
    if (s.empty()) return false;
    size_t start = (s[0] == '-' ? 1 : 0);
    return std::all_of(s.begin() + start, s.end(), ::isdigit);
}

bool MotionCoordinator::is_bool_(const std::string& s) {
    std::string lower;
    std::transform(s.begin(), s.end(), std::back_inserter(lower), ::tolower);

    return (lower == "true" || lower == "false" || s == "1" || s == "0");
}

bool MotionCoordinator::sto_bool_(const std::string& s) {
    std::string lower;
    std::transform(s.begin(), s.end(), std::back_inserter(lower), ::tolower);

    if (lower == "true" || lower == "1")  return true;
    if (lower == "false" || lower == "0") return false;

    LOG_ERROR("Invalid boolean string: %s", s.c_str());

    return false;
}

bool MotionCoordinator::is_float_(const std::string& s) {
    if (s.empty()) return false;

    std::istringstream iss(s);
    float val;
    iss >> std::noskipws >> val;

    return !iss.fail() && iss.eof();
}