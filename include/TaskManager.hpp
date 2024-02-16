#ifndef TaskManager_HPP
#define TaskManager_HPP

#include <vector>
#include <functional>
#include "pico/stdlib.h"

using namespace std;

struct Task{
    
    bool status;
    function<bool()> func;

    unsigned int interval;

    absolute_time_t lastRunTime;
    

    Task(function<bool()> func, int interval ) 
    : func(func), interval(interval), lastRunTime(get_absolute_time()), status(true){}

    void excecute(){

        if (func){
            status = func();
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

            do{
                allDone = true;

                now = get_absolute_time();

                for (Task& task:task_list){

                    if (task.status){
                        if(absolute_time_diff_us(task.lastRunTime,now)>=task.interval){
                            task.excecute();
                            task.lastRunTime=now;
                        }
                    }
                    
                    allDone &= !task.status;
                }

                sleep_us(10);

                
            } while(!allDone);

        }

    private:
        vector<Task> task_list;

};


#endif