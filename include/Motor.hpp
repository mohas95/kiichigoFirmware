#ifndef Motor_HPP
#define Motor_HPP

#include "StepperMotorDriver.hpp"


class Motor {
    public:
        virtual void test(){}
};

class StepperMotor : public Motor{
    // revolutions per second

    public:
        StepperMotor(StepperMotorDriver* driver, int steps_per_rev) 
        : driver(driver), steps_per_rev(steps_per_rev){

        }


        void action(float speed, int steps, bool direction){
            reset_activity();
            set_speed(speed);
            driver->set_direction(direction);
            set_steps(steps);
        }


        void set_speed(float revs_per_second){

            delay_per_pulse = (revs_per_second / steps_per_rev) * 1000000; // microseconds per step

        }

        bool step(){
            if (queued_steps != 0){
                driver -> step_pulse();
                update_step();

                return true;
            }else{
                return false;
            }
            
        }

        // void action_reader(){}


    private:

        unsigned int  steps_per_rev;
        StepperMotorDriver* driver;
        int queued_steps = 0;
        int step_tracker = 0;
        float rev_tracker = 0;
        // int steps_left;
        double delay_per_pulse = 5000; //microseconds
        float steps_per_second;
        float mm_per_rev;

        void reset_activity(){
            queued_steps=0;
            step_tracker=0;
            rev_tracker=0;
        }

        void set_steps(int steps){
            queued_steps = steps;
        }

        void update_step(){
            queued_steps--;
            step_tracker++;
        }

    
};

#endif