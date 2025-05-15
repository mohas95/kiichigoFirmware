#ifndef MotionPlanner_H
#define MotionPlanner_H
#include "StepperMotor.h"
#include <vector>
#include <unordered_map>
#include <queue>
#include <functional>
#include <sstream>
#include <string.h>

struct MotionConfig{
    std::vector<StepperMotor*> stepper_motors={};
};


class MotionPlanner{
    public:

        MotionPlanner(const MotionConfig& config);

        std::string read_serial_line();

        bool update_actions();

        void loop_forever();

        void request_action();
    
    private:

        void register_commands_();
        bool is_integer_(const std::string& s);
        bool is_bool_(const std::string& s);
        bool sto_bool_(const std::string& s);


        std::queue<std::function<void()>> action_queue_;
        std::unordered_map<std::string, StepperMotor*> stepper_motors_;
        std::unordered_map<std::string, std::function<void(std::istringstream&)>> command_handlers_;
        

};




#endif