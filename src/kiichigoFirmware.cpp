#include <stdio.h>
#include "pico/stdlib.h"

#include "Log.h"
#include "TB67S128FTG.h"
#include "StepperMotor.h"
// #include "Scheduler.h"


uint32_t step_freq = 500;

int main()
{
    stdio_init_all();

    TB67S128FTG stepper_driver1(0, 1, 2, 3, 4, 5);

    stepper_driver1.set_pulse_interval(5000);

    StepperMotor stepper1("x-axis", stepper_driver1, 200);


    // stepper1.step_for(1000);
    stepper1.revolve(5);


    while (true) {

        stepper_driver1.step_pulse();      

    }

    return 0;
}
