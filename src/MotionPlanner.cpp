#include "MotionPlanner.h"
#include "Log.h"
#include <stdio.h>
#include <algorithm>
#include <cctype>


MotionPlanner::MotionPlanner(const MotionConfig& config) {
    
    for (StepperMotor* stepper : config.stepper_motors){
        stepper_motors_[stepper->label()] = stepper; 
    }

    for (LimitSwitch* limit_switch : config.limit_switches){
        limit_switches_[limit_switch->label()]=limit_switch;
    }

    register_commands_();

}

void MotionPlanner::output_states() {

    std::string state_str;



}

void MotionPlanner::request_limit_switch_action(){

    for (const auto&[label, limit_switch]: limit_switches_){
        
        if(limit_switch->get_state()){
            
            std::ostringstream motors_to_stop;
            
            for(std::string motor_label : limit_switch->get_mapping()) {

                motors_to_stop << motor_label << "," << limit_switch->get_fixed_position() <<" ";

            }

            std::istringstream iss(motors_to_stop.str());

            command_handlers_["HIT"](iss);
        }
    }

}



std::string MotionPlanner::read_serial_line() {
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


void MotionPlanner::request_serial_action(){

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

bool MotionPlanner::update_actions(){

    bool active_flag=false;

    for (const auto&[label, motor]: stepper_motors_){
        if(motor->active()){
            motor->step();
            active_flag=true;
        }
    }

    return active_flag;

}


void MotionPlanner::loop_forever(){
    
    while (true){
        
        request_limit_switch_action();
        request_serial_action();

        while(!action_queue_.empty()){

            auto action = action_queue_.front();
            action_queue_.pop();
            action();

            
            while(true){

                request_limit_switch_action();

                if(!interupt_flag_){
                    request_serial_action();
                }

                bool busy = update_actions();

                if(!busy){break;}

            }

        }

        interupt_flag_= false;
        if(!disable_action_for_.empty()){disable_action_for_.clear();}

        sleep_ms(1);

        
    }

}

/*Private Methods and  Helper functions*/

void MotionPlanner::register_commands_(){

    command_handlers_["MOVE"] = [&](std::istringstream& iss) {
        /*This command parses commands from serial with this format:
            "MOVE X,10.0 Y,100.0 Z,-100.0"
            This is an QUEUED command 
        */
        std::string full_line = iss.str().substr(iss.tellg());
        std::string token;
        std::unordered_map<std::string, double> command_dict;
        while (iss >> token){

            if (token.length() <2 ) {continue;}

            std::istringstream command_stream(token);
            std::string label, value_str;
            
            if(std::getline(command_stream, label, ',') && std::getline(command_stream, value_str)){

                if(stepper_motors_.find(label) == stepper_motors_.end()){
                    LOG_WARN("Unknown motor: %s\n",label.c_str());
                    continue;
                }

                if (value_str.empty() || (!is_float_(value_str))) {
                    LOG_WARN("Invalid input: %s\n", value_str.c_str());
                    continue;
                }

                double value = std::stod(value_str);
                command_dict[label] = value;

            }else{
                LOG_WARN("Invalid token: %s\n", token.c_str());
                continue;
            }
        }

        if(!command_dict.empty()){

            action_queue_.push([command_dict, this](){

                for(const auto& [label, value] : command_dict){

                    stepper_motors_[label]->revolve(value);
                    LOG_INFO("Action: MOVE %s -> %0.2f revolutions\n", label.c_str(), value);

                }
            });

            LOG_INFO("Added to Queue: MOVE %s\n", full_line.c_str());

        }else{
            LOG_WARN("No valid actions in MOVE command: %s\n", full_line.c_str());
        }

    };

    
    command_handlers_["SPEED"] = [&](std::istringstream& iss) {
        /*This command parses commands from serial with this format:
            "SPEED X,200.0 Y,100.0 Z,300.0" will take negative values as absolute value
            This is an QUEUED command 
        */
        std::string full_line = iss.str().substr(iss.tellg());
        std::string token;
        std::unordered_map<std::string, double> command_dict;

        while(iss>>token){

            if (token.length() <2 ) {continue;}

            std::istringstream command_stream(token);
            std::string label, value_str;

            if(std::getline(command_stream, label, ',') && std::getline(command_stream, value_str)){
                if(stepper_motors_.find(label) == stepper_motors_.end()){
                    LOG_WARN("Unknown motor: %s\n",label.c_str());
                    continue;
                }

                if (value_str.empty() || (!is_float_(value_str))) {
                    LOG_WARN("Invalid input: %s\n", value_str.c_str());
                    continue;
                }

                // uint32_t value = static_cast<uint32_t>(std::abs(std::stoi(value_str)));
                double value = std::abs(std::stod(value_str));
                command_dict[label] = value;

            }else{
                LOG_WARN("Invalid token: %s\n", token.c_str());
                continue;
            }
        }

        if(!command_dict.empty()){

            action_queue_.push([command_dict, this](){

                for(const auto& [label, value] : command_dict){

                    stepper_motors_[label]->set_speed(value);
                    LOG_INFO("Action: SPEED %s -> %0.2f rpm\n", label.c_str(), value);

                }
            });

            LOG_INFO("Added to Queue: SPEED %s\n", full_line.c_str());

        }else{
            LOG_WARN("No valid actions in SPEED command: %s\n", full_line.c_str());
        }

    };

    command_handlers_["STANDBY"] = [&](std::istringstream& iss) {
        /*This command parses commands from serial with this format:
            "STANDBY X,0 Y,1 Z,0"
            This is an QUEUED command 

        */
        std::string full_line = iss.str().substr(iss.tellg());
        std::string token;
        std::unordered_map<std::string, bool> command_dict;

        while(iss>>token){

            if (token.length() <2 ) {continue;}

            std::istringstream command_stream(token);
            std::string label, value_str;

            if(std::getline(command_stream, label, ',') && std::getline(command_stream, value_str)){
                if(stepper_motors_.find(label) == stepper_motors_.end()){
                    LOG_WARN("Unknown motor: %s\n",label.c_str());
                    continue;
                }

                if (value_str.empty() || (!is_bool_(value_str))) {
                    LOG_WARN("Invalid input: %s\n", value_str.c_str());
                    continue;
                }

                bool value = sto_bool_(value_str);
                command_dict[label] = value;

            }else{
                LOG_WARN("Invalid token: %s\n", token.c_str());
                continue;
            }
        }

        if(!command_dict.empty()){

            action_queue_.push([command_dict, this](){

                for(const auto& [label, value] : command_dict){

                    stepper_motors_[label]->set_standbyMode(value);
                    LOG_INFO("Action: STANDBY %s -> %s\n", label.c_str(), value ? "enabled": "disabled");

                }
            });
            LOG_INFO("Added to Queue: STANDBY %s\n", full_line.c_str());

        }else{
            LOG_WARN("No valid actions in STANDBY command: %s\n", full_line.c_str());
        }

    };


    command_handlers_["HIT"] = [&](std::istringstream& iss) {
        /*This command parses commands from serial with this format:
            "HIT X,10.0 Y,200.5 Z,-30.0"
            This is an INTERUPT command 
        */
        std::string full_line = iss.str().substr(iss.tellg());
        std::string token;
        std::unordered_map<std::string, double> command_dict;

        while(iss>>token){

            if (token.length() <2 ) {continue;}

            std::istringstream command_stream(token);
            std::string label, value_str;

            if(std::getline(command_stream, label, ',') && std::getline(command_stream, value_str)){
                if(stepper_motors_.find(label) == stepper_motors_.end()){
                    LOG_WARN("Unknown motor: %s\n",label.c_str());
                    continue;
                }

                if (value_str.empty() || (!is_float_(value_str))) {
                    LOG_WARN("Invalid input: %s\n", value_str.c_str());
                    continue;
                }

                double value = std::stod(value_str);
                command_dict[label] = value;

            }else{
                LOG_WARN("Invalid token: %s\n", token.c_str());
                continue;
            }
        }

        if(!command_dict.empty()){ //  && !interupt_flag_

            interupt_flag_=true;

            for(const auto& [label, value] : command_dict){

                if(std::find(disable_action_for_.begin(), disable_action_for_.end(), label)){ // Interupt should run once
                    LOG_WARN("INTERUPT ACTION IN PROGRESS for: %s\n", label.c_str());
                }else{

                    stepper_motors_[label]->update_position(value);

                    if(stepper_motors_[label]->get_direction()){
                        stepper_motors_[label]->revolve(-1); 
                    }else{
                        stepper_motors_[label]->revolve(1); 
                    }
                    disable_action_for_.push_back(label); //once ran disable so that it doesnt interfere

                    LOG_INFO("INTERUPT: HIT %s,%0.2f\n", label.c_str(),value);

                }

            }

            // Clear pending commands
            std::queue<std::function<void()>> empty;
            std::swap(action_queue_, empty);

            // Flush any buffered serial input
            // while (getchar_timeout_us(0) != PICO_ERROR_TIMEOUT) {}

        }else{
            LOG_WARN("No valid actions in HIT command: %s\n", full_line.c_str());
        }

    };


    command_handlers_["STOP"] = [&](std::istringstream& iss) {
        /*This command parses commands from serial with this format:
            "STOP X Y Z"
            This is an INTERUPT command 
        */
        std::string full_line = iss.str().substr(iss.tellg());
        std::string token;
        std::vector<std::string> command_vec;

        while(iss>>token){

            if (token.length() <1 ) {
                LOG_WARN("Invalid token: %s\n", token.c_str());
                continue;
            }

            std::string label=token;
            
            if(stepper_motors_.find(label) == stepper_motors_.end()){
                LOG_WARN("Unknown motor: %s\n",label.c_str());
                continue;
            }

            command_vec.push_back(label);

        }

        if(!command_vec.empty()){

            for(const auto& label : command_vec){
                stepper_motors_[label]->revolve(0); // sets all steps to zero
                stepper_motors_[label]->update_position();
            }

            // Clear pending commands
            std::queue<std::function<void()>> empty;
            std::swap(action_queue_, empty);

            // Flush any buffered serial input
            while (getchar_timeout_us(0) != PICO_ERROR_TIMEOUT) {}

            LOG_INFO("INTERUPT: STOP %s\n", full_line.c_str());

        }else{
            LOG_WARN("No valid actions in STOP command: %s\n", full_line.c_str());
        }

    };

}



/*Helper functions*/

bool MotionPlanner::is_integer_(const std::string& s) {
    if (s.empty()) return false;
    size_t start = (s[0] == '-' ? 1 : 0);
    return std::all_of(s.begin() + start, s.end(), ::isdigit);
}

bool MotionPlanner::is_bool_(const std::string& s) {
    std::string lower;
    std::transform(s.begin(), s.end(), std::back_inserter(lower), ::tolower);

    return (lower == "true" || lower == "false" || s == "1" || s == "0");
}

bool MotionPlanner::sto_bool_(const std::string& s) {
    std::string lower;
    std::transform(s.begin(), s.end(), std::back_inserter(lower), ::tolower);

    if (lower == "true" || lower == "1")  return true;
    if (lower == "false" || lower == "0") return false;

    LOG_ERROR("Invalid boolean string: %s", s.c_str());

    return false;
}

bool MotionPlanner::is_float_(const std::string& s) {
    if (s.empty()) return false;

    std::istringstream iss(s);
    float val;
    iss >> std::noskipws >> val;

    return !iss.fail() && iss.eof();
}