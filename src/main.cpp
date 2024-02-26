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

Task create_stepper_task(StepperMotor& stepper_motor, float speed, int steps, bool direction, unsigned int step_mode = 1){
    // this is passing a preset setup function for the test, so that i can store the task
    auto setup_func = [&stepper_motor, speed, steps, direction, step_mode](){
        stepper_motor.action(speed, steps, direction, step_mode);
        
        return stepper_motor.get_delay_per_pulse();
    };

    auto func = [&stepper_motor](){
        bool status = stepper_motor.step();
        return status;
    };
    
    Task task(func, setup_func);

    return task;
}

Task create_stepper_task(StepperMotor& stepper_motor, bool stby_mode){
    // this is passing a preset setup function for the test, so that i can store the task
    auto setup_func = [&stepper_motor, stby_mode](){
        stepper_motor.action(stby_mode);

        return stepper_motor.get_delay_per_pulse();
    };

    auto func = [&stepper_motor](){
        bool status = stepper_motor.step();
        return status;
    };
    
    Task task(func, setup_func);

    return task;
}
 

int main(){
    stdio_init_all();

    Scheduler motor_scheduler;
    TB67S128FTG md1(0, 1, 2, 3, 4, 5, TB67S128FTG::QUARTER_STEP);

    vector<Task> task_list;
    
    StepperMotor stepper1(&md1, 200, 1, 180);

    task_list.push_back(create_stepper_task(stepper1, 10, 800, true, 1));
    task_list.push_back(create_stepper_task(stepper1, 10, 800*4, true, 4));
    task_list.push_back(create_stepper_task(stepper1, 10, 800*32, true, 32));
    task_list.push_back(create_stepper_task(stepper1, 10, 800*128, true, 128));
    task_list.push_back(create_stepper_task(stepper1, true));


    // for (int i=5;i<=180; i+=5){

    //     task_list.push_back(create_stepper_task(stepper1, i, 800, true, ));

    // }

    // for (int i=180;i>=5; i-=5){

    //     task_list.push_back(create_stepper_task(stepper1, i, 800, false));

    // }

    for (Task& task: task_list){

        motor_scheduler.add_task(task);
        motor_scheduler.run();
    }

    
    return 0;
};