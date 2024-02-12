#ifndef Motor
#define Motor

#include "StepperMotorDriver.hpp"


class Motor{
    public:

};

class StepperMotor : public Motor {
    // revolutions per second

    public:
        StepperMotor(StepperMotorDriver* driver, int steps_per_rev) 
        : driver(driver), steps_per_rev(steps_per_rev){

        }


        void set_speed(){

        }


    private:

        unsigned int  steps_per_rev;
        StepperMotorDriver* driver;
        int queued_steps;
        int steps_left;
        float delay_for_speed;

        int revs_to_steps(int revs){
            
        }

    
    
};

#endif