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
    
    StepperMotor stepper1(&md1, 200);

    Task task1 = create_stepper_task(stepper1, 1,1000, true);
    Task task2  = create_stepper_task(stepper1, 1, 1000, false);    

    motor_scheduler.add_task(task1);
    motor_scheduler.run();

    motor_scheduler.add_task(task2);
    motor_scheduler.run();

    return 0;
};