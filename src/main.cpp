#include "StepperMotorDriver.hpp"
#include "Motor.hpp"
#include "TaskManager.hpp"
#include "pico/stdlib.h"
#include <string>
#include <memory>
#include <vector>
#include <functional>

using namespace std;


Task monitor_steppers_task(vector<StepperMotor*> motor_list){
    // this is passing a preset setup function for the test, so that i can store the task
    // auto setup_func = [&stepper_motor, stby_mode](){
    //     stepper_motor.action(stby_mode);

    //     return stepper_motor.get_delay_per_pulse();
    // };

    auto func = [motor_list](){
        bool status = true;

        string out = "";

        for (StepperMotor* motor : motor_list){
            string motor_out= to_string(motor->get_step_tracker());
            if (!out.empty()){
                out +=",";
            }

            out+=motor_out;
        } 

        printf("%s\n", out.c_str());

        return status;
    };
    
    Task task(func,nullptr,1000000);

    return task;
}


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

    

    // for (Task& task: task_list){

    //     motor_scheduler.add_task(task);
    // }

    motor_scheduler.add_to_queue(task_list1);
    motor_scheduler.add_to_queue(task_list2);
    motor_scheduler.add_to_queue(task_list3);



    background_tasks.push_back(monitor_steppers_task(stepper_list));

    for (Task& task: background_tasks){

        motor_scheduler.add_background_task(task);
    }

    // motor_scheduler.begin();
    // motor_scheduler.run();

    motor_scheduler.loop();

    // task_list.push_back(create_stepper_task(stepper1, true));
    // task_list.push_back(create_stepper_task(stepper2, true));
    // task_list.push_back(create_stepper_task(stepper3, true));

    // for (Task& task: task_list){

    //     motor_scheduler.add_task(task);
    // }

    // motor_scheduler.run();

    printf("Done!\n");


    
    return 0;
};