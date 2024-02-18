#ifndef Motor_HPP
#define Motor_HPP

#include "StepperMotorDriver.hpp"


class Motor {
    public:
        virtual void action(float speed, int steps, bool direction)=0;
};

class StepperMotor : public Motor{
    // revolutions per second
    // Speed is in revolutions per minute RPM

    public:
        StepperMotor(StepperMotorDriver* driver, int steps_per_rev,  int min_speed = 60, int max_speed = 180) 
        : driver(driver), steps_per_rev(steps_per_rev), min_speed(min_speed), max_speed(max_speed){}

        void action(float speed, int steps, bool direction) override{
            reset_activity();
            set_speed(speed);
            driver->set_direction(direction);
            set_steps(steps);
        }

        void set_speed(float speed_RPM){

            if (speed_RPM >= max_speed){
                speed_RPM = max_speed;
            } else if(speed_RPM <= min_speed){
                speed_RPM = min_speed;
            }

            float revs_per_second = speed_RPM/60;

            delay_per_pulse =  1000000 / (steps_per_rev*revs_per_second); // microseconds per step

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
            return delay_per_pulse; // microseconds per step
        }

        int get_delay_per_pulse(float speed_RPM){

            if (speed_RPM >= max_speed){
                speed_RPM = max_speed;
            } else if(speed_RPM <= min_speed){
                speed_RPM = min_speed;
            }

            float revs_per_second = speed_RPM/60;
            
            return 1000000 / (steps_per_rev*revs_per_second ); // microseconds per step
        }
        


    private:
        unsigned int  steps_per_rev;
        StepperMotorDriver* driver;
        int queued_steps = 0;
        int step_tracker = 0;
        float rev_tracker = 0;
        int delay_per_pulse = 5000; // microseconds
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