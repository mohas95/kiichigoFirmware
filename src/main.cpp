#include "StepperMotorDriver.hpp"
#include "Motor.hpp"
#include "TaskManager.hpp"
#include "pico/stdlib.h"
#include <string>
#include <memory>

using namespace std;


Task create_stepper_task(StepperMotor& stepper_motor, float speed, int steps, bool direction){

    stepper_motor.action(speed, steps, direction);

    auto func = [&stepper_motor](){
        bool status = stepper_motor.step();
        
        return status;
    };
    
    Task task(func, stepper_motor.get_delay_per_pulse());

    return task;

}
 

int main(){

    stdio_init_all();

    Scheduler motor_scheduler;

    TB67S128FTG md1(0, 1, 2, 3, 4, 5);
    
    StepperMotor stepper1(&md1, 200);

    Task task1 = create_stepper_task(stepper1, 1,1000, true);

    motor_scheduler.add_task(task1);
    
    // stepper1.action(1, 1000, true);

    // absolute_time_t now;
    // absolute_time_t stepper1_task = get_absolute_time();
    // absolute_time_t stepper2_task = get_absolute_time();

    motor_scheduler.run();
    
    Task task2  = create_stepper_task(stepper1, 1, 1000, false);
    motor_scheduler.add_task(task2);
    motor_scheduler.run();

    return 0;

};