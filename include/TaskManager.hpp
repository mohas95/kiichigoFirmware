#ifndef TaskManager_HPP
#define TaskManager_HPP

#include <vector>
#include <functional>
#include "pico/stdlib.h"

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

    private:
        vector<reference_wrapper<Task>> task_list;

};


#endif