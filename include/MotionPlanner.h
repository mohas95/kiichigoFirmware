#ifndef MotionPlanner_H
#define MotionPlanner_H
#include "StepperMotor.h"
#include <vector>
#include <unordered_map>


#define BUFFER_SIZE 64

struct MotionConfig{
    std::vector<StepperMotor*> stepper_motors={};
};


class MotionPlanner{
    public:

        MotionPlanner(const MotionConfig& config);

        bool start_action();

        void update_action();

        void loop_forever();

        bool stop();

        void request_action();

        void load_action_queue();

        void process_command(const char* line);
    
    private:
        bool action_queue_;
        char input_buffer[BUFFER_SIZE];
        // std::vector<StepperMotor*> stepper_motors_;
        std::unordered_map<std::string, StepperMotor*> stepper_motors_;
        

};




#endif