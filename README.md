# kiichigoFirmware

### Project Description
The goal of this project is to make a stepper motor motion firmware for RP2040-based microcontroller. This will include: 
- a user-friendly library for abstracting stepper motor control for major stepper motor drivers.
- Simple motion planning for synchronous multi-axis machines such as CNC or 3D Printer
- Compatibility with G-code

## Example usage
### motor library usage
```cpp
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
```

## Feedback
All kinds of feedback and contributions are welcome.
- [Create an issue](https://github.com/moha7108/kiichigoFirmware/issues)
- [Create a pull request](https://github.com/moha7108/kiichigoFirmware/pulls)
- reach out to @moha7108

## Contributors
- Mohamed Debbagh
    - [github](https://github.com/moha7108/), [website](https://moha7108.github.io/)

## Change Log
### 0.0.1 (Not released yet)
- initial stepper motor driver library
    - StepperMotorDriver class/header as virtual parent class with inheritance of specific drivers:
        - support for TB67S128FTG stepper motor driver added
    - Motor class/header as virtual parent class with inheritance for specific motor interface:
        - stepper motor interface defined
- TaskManager Header, created to define multitasking and scheduling interface
    - Task struct which defines tasks for scheduler class to run simultaneously
    - scheduler class defined to run tasks until each are completed for sequential task operation 

