#include "StepperMotorDriver.hpp"
#include "Motor.hpp"
#include "TaskManager.hpp"
#include "pico/stdlib.h"
#include <string>
#include <memory>
#include <vector>
#include <functional>

using namespace std;

unsigned int working_stepModes[4] = {1,4,32,128};
 

int main(){
    stdio_init_all();

    Scheduler motor_scheduler;
    TB67S128FTG md1(0, 1, 2, 3, 4, 5, TB67S128FTG::QUARTER_STEP);

    vector<Task> task_list;
    
    StepperMotor stepper1(&md1, 200, 1, 180);

    task_list.push_back(create_stepper_task(stepper1, 90, 800, true, 1));
    task_list.push_back(create_stepper_task(stepper1, 90, 800*4, true, 4));
    task_list.push_back(create_stepper_task(stepper1, 90, 800*32, true, 32));
    task_list.push_back(create_stepper_task(stepper1, 90, 800*128, true, 128));
    task_list.push_back(create_stepper_task(stepper1, true));

    for (Task& task: task_list){

        motor_scheduler.add_task(task);
        motor_scheduler.run();
    }
    
    return 0;
};