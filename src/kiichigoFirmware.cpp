#include <stdio.h>
#include "pico/stdlib.h"

#include "Log.h"
#include "TB67S128FTG.h"
// #include "Scheduler.h"


uint32_t step_freq = 500;

int main()
{
    stdio_init_all();

    TB67S128FTG stepper1(0, 1, 2, 3, 4, 5);

    stepper1.set_pulse_interval(5000);
    stepper1.step_for(1000);


    while (true) {

        stepper1.step_pulse();

    }

    return 0;
}
