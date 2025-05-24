# kiichigoFirmware

### Project Description
The goal of this project is to make a stepper motor motion firmware for RP2040-based microcontroller. This will include: 
- a user-friendly library for abstracting stepper motor control for major stepper motor drivers.
- Simple motion planning for synchronous multi-axis machines such as CNC or 3D Printer
- Compatibility with G-code

## Example usage
### motor library usage

#### Non-looping operation (run once)
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
    vector<Task> task_list;
    vector<StepperMotor*> stepper_list;
    vector<Task> background_tasks;

    stdio_init_all();

    // Wait for USB serial to be connected
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    // Print a message to the USB serial
    printf("USB Serial connected!\n");

    Scheduler motor_scheduler;
    TB67S128FTG md1(0, 1, 2, 3, 4, 5);
    TB67S128FTG md2(6, 7, 8, 9, 10, 11);
    TB67S128FTG md3(12, 13, 14, 15, 17, 16);

    StepperMotor stepper1(&md1, 200, 1, 180);
    StepperMotor stepper2(&md2, 200, 1, 180);
    StepperMotor stepper3(&md3, 200, 1, 180);

    stepper_list.push_back(&stepper1);
    stepper_list.push_back(&stepper2);
    stepper_list.push_back(&stepper3);

    task_list.push_back(create_stepper_task(stepper1, 90, 800, true));
    task_list.push_back(create_stepper_task(stepper2, 90, 1600,false));
    task_list.push_back(create_stepper_task(stepper3, 90, 20000, true));
    
    background_tasks.push_back(STEPPER_MONITOR(stepper_list));
    
    for (Task& task: background_tasks){
        motor_scheduler.add_background_task(task);
    }

    for (Task& task: task_list){

            motor_scheduler.add_task(task);
        }

    motor_scheduler.begin();
    motor_scheduler.run();

    printf("Done!\n");

    return 0;
};
```

#### Loop operation (continuous operation)
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
    vector<Task> task_list1;
    vector<Task> task_list2;
    vector<Task> task_list3;

    vector<Task> background_tasks;

    vector<StepperMotor*> stepper_list;

    stdio_init_all();

    // Wait for USB serial to be connected
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    // Print a message to the USB serial
    printf("USB Serial connected!\n");

    Scheduler motor_scheduler;
    TB67S128FTG md1(0, 1, 2, 3, 4, 5);
    TB67S128FTG md2(6, 7, 8, 9, 10, 11);
    TB67S128FTG md3(12, 13, 14, 15, 17, 16);
    
    StepperMotor stepper1(&md1, 200, 1, 180);
    StepperMotor stepper2(&md2, 200, 1, 180);
    StepperMotor stepper3(&md3, 200, 1, 180);

    stepper_list.push_back(&stepper1);
    stepper_list.push_back(&stepper2);
    stepper_list.push_back(&stepper3);


    task_list1.push_back(create_stepper_task(stepper1, 90, 800, true));
    task_list1.push_back(create_stepper_task(stepper2, 90, 1600,false));
    task_list1.push_back(create_stepper_task(stepper3, 90, 20000, true));

    task_list2.push_back(create_stepper_task(stepper1, 90, 800, false));
    task_list2.push_back(create_stepper_task(stepper2, 90, 1600,true));
    task_list2.push_back(create_stepper_task(stepper3, 90, 20000, false));

    task_list3.push_back(create_stepper_task(stepper1, true));
    task_list3.push_back(create_stepper_task(stepper2, true));
    task_list3.push_back(create_stepper_task(stepper3, true));

    motor_scheduler.add_to_queue(task_list1);
    motor_scheduler.add_to_queue(task_list2);
    motor_scheduler.add_to_queue(task_list3);


    background_tasks.push_back(STEPPER_MONITOR(stepper_list));

    for (Task& task: background_tasks){
        motor_scheduler.add_background_task(task);
    }

    

    motor_scheduler.loop();
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
        - STEPPER_MONITOR() for querying stepper motors
    - background/daemon tasks for call_back functionality and continuous operations
    - Queued task handling
    - loop() which will run the event handling forever.