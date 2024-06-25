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

unsigned int working_stepModes[4] = {1,4,32,128};
 
int main(){
    stdio_init_all();

    Scheduler motor_scheduler;
    TB67S128FTG md1(0, 1, 2, 3, 4, 5);
    TB67S128FTG md2(6, 7, 8, 9, 10, 11);
    TB67S128FTG md3(12, 13, 14, 15, 17, 16);



    vector<Task> task_list;
    
    StepperMotor stepper1(&md1, 200, 1, 180);

    task_list.push_back(create_stepper_task(stepper1, 90, 800, true, 1));
    task_list.push_back(create_stepper_task(stepper1, 90, 800*4, true, 4));
    task_list.push_back(create_stepper_task(stepper1, 90, 800*32, true, 32));
    task_list.push_back(create_stepper_task(stepper1, 90, 800*128, true, 128));
    task_list.push_back(create_stepper_task(stepper1, true));
    task_list.push_back(create_stepper_task(stepper1, 50, 800*4, false, 4));
    task_list.push_back(create_stepper_task(stepper1, 50, 800*4, true, 4));
    task_list.push_back(create_stepper_task(stepper1, true));


    for (Task& task: task_list){

        motor_scheduler.add_task(task);
        motor_scheduler.run();
    }
    
    return 0;
};
```

## Feedback
All kinds of feedback and contributions are welcome.
- [Create an issue](https://github.com/mohas95/kiichigoFirmware/issues)
- [Create a pull request](https://github.com/mohas95/kiichigoFirmware/pulls)
- reach out to @mohas95

## Contributors
- Mohamed Debbagh
    - [github](https://github.com/mohas05/), [website](https://mohas95.github.io/)

## Change Log
### 0.0.1 (Not released yet)
- initial stepper motor driver library
    - StepperMotorDriver class/header as virtual parent class with inheritance of specific drivers:
        - support for TB67S128FTG stepper motor driver added
    - Motor class/header as virtual parent class with inheritance for specific action based motor interface:
        - stepper motor interface defined
- TaskManager Header, created to define multitasking and scheduling interface
    - Task struct which defines tasks for scheduler class to run simultaneously
    - scheduler class defined to run tasks until each are completed for sequential task operation 
    - utility functions for creating tasks for specific use cases
        - create_stepper_task()