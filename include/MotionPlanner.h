#ifndef MotionPlanner_H
#define MotionPlanner_H
#include "StepperMotor.h"
#include "LimitSwitch.h"
#include <vector>
#include <unordered_map>
#include <queue>
#include <functional>
#include <sstream>
#include <string.h>

struct MotionConfig{
    std::vector<StepperMotor*> stepper_motors={};
    std::vector<LimitSwitch*> limit_switches={};
};


class MotionPlanner{
    public:

        MotionPlanner(const MotionConfig& config);

        std::string read_serial_line();
        void request_limit_switch_action();
        void output_states();
        bool update_actions();
        void loop_forever();
        void request_serial_action();
    
    private:

        void register_commands_();
        bool is_integer_(const std::string& s);
        bool is_bool_(const std::string& s);
        bool is_float_(const std::string& s);
        bool sto_bool_(const std::string& s);


        std::queue<std::function<void()>> action_queue_;
        std::unordered_map<std::string, StepperMotor*> stepper_motors_;
        std::unordered_map<std::string, LimitSwitch*> limit_switches_;
        std::unordered_map<std::string, std::function<void(std::istringstream&)>> command_handlers_;        
        bool interupt_flag_;

};




#endif