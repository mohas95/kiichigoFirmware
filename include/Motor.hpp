#ifndef Motor_HPP
#define Motor_HPP

#include "StepperMotorDriver.hpp"

class Motor {
    public:
        virtual void action(float speed, int steps, bool direction, unsigned int step_mode)=0;
};

class StepperMotor : public Motor{
    // Speed is in revolutions per minute RPM
    public:
        StepperMotor(StepperMotorDriver* driver, int steps_per_rev,  int min_speed = 60, int max_speed = 180) 
        : driver(driver), steps_per_rev(steps_per_rev), min_speed(min_speed), max_speed(max_speed){
            step_multiplier = driver->get_step_mode();
            unsigned int mid_speed = (max_speed - min_speed)/2;
            set_speed(mid_speed); //Default speed
        }

        void action(float speed, int steps, bool direction, unsigned int step_mode = false) override{
            reset_activity();
            if(driver->get_standby_mode()){
                driver->set_standby_mode(false);
            }
            if (step_mode){
                driver->set_step_mode(step_mode);
                step_multiplier = driver->get_step_mode();
            }
            set_speed(speed);
            driver->set_direction(direction);
            set_steps(steps);
        }

        void action(bool stby_mode){
            reset_activity();
            if(stby_mode != driver->get_standby_mode()){
                driver->set_standby_mode(stby_mode);
            }
        }

        void set_speed(float speed_RPM){

            if (speed_RPM >= max_speed){
                speed_RPM = max_speed;
            } else if(speed_RPM <= min_speed){
                speed_RPM = min_speed;
            }

            float revs_per_second = speed_RPM/60;

            delay_per_pulse =  1000000 / (steps_per_rev*step_multiplier*revs_per_second); // microseconds per step
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

        int get_delay_per_pulse(float speed_RPM, unsigned int step_mode){

            if (speed_RPM >= max_speed){
                speed_RPM = max_speed;
            } else if(speed_RPM <= min_speed){
                speed_RPM = min_speed;
            }

            float revs_per_second = speed_RPM/60;
            
            return 1000000 / (steps_per_rev*step_mode*revs_per_second ); // microseconds per step
        }

        unsigned int get_stepsPerRevolution(){
            return steps_per_rev * step_multiplier;
        }

        unsigned int get_stepsPerRevolution(unsigned int step_mode){
            return steps_per_rev *step_mode;
        }

    private:
        StepperMotorDriver* driver;
        unsigned int  steps_per_rev;
        unsigned int step_multiplier;
        int queued_steps = 0;
        int step_tracker = 0;
        float rev_tracker = 0;
        int delay_per_pulse; // microseconds
        int min_speed;
        int max_speed;

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