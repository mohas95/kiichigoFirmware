#ifndef Motor_HPP
#define Motor_HPP

#include "StepperMotorDriver.hpp"


class Motor {
    public:
        virtual void test(){}
        virtual void action(float speed, int steps, bool direction)=0;
};

class StepperMotor : public Motor{
    // revolutions per second

    public:
        StepperMotor(StepperMotorDriver* driver, int steps_per_rev, int min_speed = 1, int max_speed = 3) 
        : driver(driver), steps_per_rev(steps_per_rev), min_speed(min_speed), max_speed(max_speed){}

        void action(float speed, int steps, bool direction) override{
            reset_activity();
            set_speed(speed);
            driver->set_direction(direction);
            set_steps(steps);
        }

        void set_speed(float revs_per_second){

            if (revs_per_second >= max_speed){
                revs_per_second = max_speed;
            } else if(revs_per_second <= min_speed){
                revs_per_second = min_speed;
            }

            delay_per_pulse =  1000000 / (steps_per_rev*revs_per_second ); // microseconds per step

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

        int get_delay_per_pulse(){
            return delay_per_pulse;
        }

        int get_delay_per_pulse(float revs_per_second){

            if (revs_per_second >= max_speed){
                revs_per_second = max_speed;
            } else if(revs_per_second <= min_speed){
                revs_per_second = min_speed;
            }
            
            return 1000000 / (steps_per_rev*revs_per_second ); // microseconds per step
        }
        


    private:

        unsigned int  steps_per_rev;
        StepperMotorDriver* driver;
        int queued_steps = 0;
        int step_tracker = 0;
        float rev_tracker = 0;
        // int steps_left;
        int delay_per_pulse = 5000; //microseconds
        float steps_per_second;

        int min_speed;
        int max_speed;
        // float mm_per_rev;

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