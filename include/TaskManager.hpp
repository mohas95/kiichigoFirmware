#ifndef TaskManager_HPP
#define TaskManager_HPP

#include <vector>
#include <functional>
#include "pico/stdlib.h"
#include "Motor.hpp"

using namespace std;

struct Task{
    bool status;
    function<bool()> func; //func expects to return a bool value with the status of the motor's task
    function<int()> setup_func; //setup_func expects to return an int with the interval after all params setup 
    unsigned int interval; // if set manually the setup_func will not use its expected value
    absolute_time_t lastRunTime;

    Task(function<bool()> func, function<int()> setup_func = nullptr, int interval = false) 
    : func(func), setup_func(setup_func), interval(interval), lastRunTime(get_absolute_time()), status(true){}

    void execute(){
        if (func){
            status = func();
        }
    }

    void begin(){
        if (setup_func){
            if(!interval){
                interval = setup_func();
            }else{
                setup_func();
            }
        }
    }
};

class Scheduler {
    public:
        void add_task(Task& task){
            task_list.push_back(task);
        }

        void add_background_task(Task& task){
            background_tasks.push_back(task);
        }

        void add_to_queue(vector<Task>& task_list){

            queue.push_back(task_list);
            
        }

        void run(){

            bool allDone;
            absolute_time_t now;

            // Run setup code once before running interval task
            for (Task& task: task_list){
                if (task.status){
                    task.begin();
                }
            }

            do{
                allDone = true;

                now = get_absolute_time();

                run_background(now);

                for (Task& task:task_list){

                    if (task.status){
                        if(absolute_time_diff_us(task.lastRunTime,now)>=task.interval){
                            task.execute();
                            task.lastRunTime=now;
                        }
                    }
                    
                    allDone &= !task.status;
                }

                sleep_us(10);

                
            } while(!allDone);
        
        task_list.clear();

        }

        void run_background(absolute_time_t now){

            for (Task& task:background_tasks){
                    if (task.status){
                        if(absolute_time_diff_us(task.lastRunTime,now)>=task.interval){
                            task.execute();
                            task.lastRunTime=now;
                        }
                    }
                }

        }

        void begin(){

            for(Task& task : background_tasks){
                if (task.status){
                    task.begin();
                }
            }

        }

        void loop(){

            absolute_time_t now;

            begin();

            while(true){

                now = get_absolute_time();
                run_background(now);
                sleep_us(10);
                
                if(!task_list.empty()){
                    run();
                }else{
                    load_queue();
                }
            }
        }

    private:
        vector<reference_wrapper<Task>> task_list;
        vector<reference_wrapper<Task>> background_tasks;
        vector<reference_wrapper<vector<Task>>> queue;

        void load_queue(){
            if(!queue.empty()){

                vector<Task>& new_list = queue[0];

                for(Task& task:  new_list){
                    add_task(task);
                }

                queue.erase(queue.begin());
            }

        }


};

// Utility Functions
// This section is for functions specifically for generating specific tasks for special cases

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

Task STEPPER_MONITOR(vector<StepperMotor*> motor_list){

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

#endif