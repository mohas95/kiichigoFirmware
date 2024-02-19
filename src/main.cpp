#include "StepperMotorDriver.hpp"
#include "Motor.hpp"
#include "TaskManager.hpp"
#include "pico/stdlib.h"
#include <string>
#include <memory>
#include <vector>
#include <functional>

using namespace std;

Task create_stepper_task(StepperMotor& stepper_motor, float speed, int steps, bool direction){
    // this is passing a preset setup function for the test, so that i can store the task
    auto setup_func = [&stepper_motor, speed, steps, direction](){
        stepper_motor.action(speed, steps, direction);
    };

    auto func = [&stepper_motor](){
        bool status = stepper_motor.step();
        return status;
    };
    
    Task task(func, stepper_motor.get_delay_per_pulse(speed), setup_func);

    return task;
}
 

int main(){
    stdio_init_all();

    Scheduler motor_scheduler;
    TB67S128FTG md1(0, 1, 2, 3, 4, 5);

    vector<Task> task_list;
    
    StepperMotor stepper1(&md1, 200, 1, 180);

    for (int i=5;i<=180; i+=5){

        task_list.push_back(create_stepper_task(stepper1, i, 200, true));

    }

    for (int i=180;i>=5; i-=5){

        task_list.push_back(create_stepper_task(stepper1, i, 200, false));

    }

        for (Task& task: task_list){

        motor_scheduler.add_task(task);
        motor_scheduler.run();
    }

    
    return 0;
};