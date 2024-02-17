#ifndef TaskManager_HPP
#define TaskManager_HPP

#include <vector>
#include <functional>
#include "pico/stdlib.h"

using namespace std;

// should I turn this into a class?
struct Task{
    
    bool status;
    function<bool()> func;
    function<void()> setup_func;
    unsigned int interval;
    absolute_time_t lastRunTime;

    Task(function<bool()> func, int interval, function<void()> setup_func = nullptr) 
    : func(func), setup_func(setup_func), interval(interval), lastRunTime(get_absolute_time()), status(true){}

    void execute(){
        if (func){
            status = func();
        }
    }

    void begin(){
        if (setup_func){
            setup_func();
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