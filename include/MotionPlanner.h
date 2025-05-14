#ifndef MotionPlanner_H
#define MotionPlanner_H
#include "StepperMotor.h"
#include <vector>
#include <unordered_map>
#include <queue>
#include <functional>
#include <sstream>
#include <string.h>





#define BUFFER_SIZE 64

struct MotionConfig{
    std::vector<StepperMotor*> stepper_motors={};
};


class MotionPlanner{
    public:

        MotionPlanner(const MotionConfig& config);

        std::string read_serial_line();

        bool update_action();

        void loop_forever();

        bool stop();

        void request_action();

        // void process_command(const char* line);
    
    private:

        void register_commands_();


        std::queue<std::function<void()>> action_queue_;
        char input_buffer[BUFFER_SIZE];
        std::unordered_map<std::string, StepperMotor*> stepper_motors_;
        std::unordered_map<std::string, std::function<void(std::istringstream&)>> command_handlers_;
        

};




#endif