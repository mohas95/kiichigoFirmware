// #include "TB67S128FTG.hpp"
#include "StepperMotorDriver.hpp"
#include "pico/stdlib.h"
#include <string>
#include <memory>

using namespace std;

struct StepperMotor{
    StepperMotorDriver* motor_driver; // pointer to (&obj) instance of a child of the StepperMotorDriver Class
    int steps_per_rev;
    string name;

    // StepperMotor Constructor
    StepperMotor(StepperMotorDriver* md, int steps_per_rev = 200, string name = "SM1") 
    : motor_driver(md), steps_per_rev(steps_per_rev), name(name){}
};


int main(){

    stdio_init_all();

    TB67S128FTG md1(0, 1, 2, 3, 4, 5);
    
    StepperMotor stepper1(&md1);
    

    while (true){

        for(int i=0; i<1000; i++){
        stepper1.motor_driver -> step_pulse(true);
        sleep_us(5000);
        }

        for(int i=0; i<1000; i++){
        stepper1.motor_driver -> step_pulse(false);
        sleep_us(5000);
        }
    }
    
};