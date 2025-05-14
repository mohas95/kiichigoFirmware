#include "MotionPlanner.h"
#include "Log.h"
#include <stdio.h>
#include <algorithm>
#include <cctype>


MotionPlanner::MotionPlanner(const MotionConfig& config) {

    register_commands_();
    
    for (StepperMotor* stepper : config.stepper_motors){
        stepper_motors_[stepper->label()] = stepper; 
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


void MotionPlanner::request_action(){

    std::string line = read_serial_line();

    if (!line.empty()) {
        std::string command;
        std::istringstream iss(line);     
        iss >> command;

        std::transform(command.begin(), command.end(), command.begin(), ::toupper);

        if(command_handlers_.contains(command)){
            
            command_handlers_[command](iss);

        }else{
            LOG_WARN("unknown command: %s\n", command.c_str());
        }



    



    }
   



    
}

bool MotionPlanner::update_action(){

}


void MotionPlanner::loop_forever(){

    request_action();

    while(!action_queue_.empty()){

        action_queue_.pop();

        bool flag = true;
        while(flag){

            request_action();

            flag = update_action();

        }



    }

}



void MotionPlanner::register_commands_(){

    command_handlers_["MOVE"] = [&](std::istringstream& iss) {

        bool valid = false;
        std::string token;


        while (iss >> token){

            if (token.length() <2 ) continue;

            char label = std::tolower(token[0]);
            std::string value_str = token.substr(1);



        }

    };

    command_handlers_["SPEED"] = [&](std::istringstream& iss) {
        
    };


}