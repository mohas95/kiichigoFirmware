#include <stdio.h>
#include "pico/stdlib.h"

#include "Log.h"
#include "TB67S128FTG.h"
#include "StepperMotor.h"
// #include "Scheduler.h"

int main()
{
    stdio_init_all();

    // Wait for USB serial to be connected
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    // Print a message to the USB serial
    printf("USB Serial connected!\n");

    TB67S128FTG stepper_driver1(0, 1, 2, 3, 4, 5, StepperDriver::StepMode::HALF);

    // stepper_driver1.set_pulse_interval(5000); 

    StepperMotor stepper1("x-axis", stepper_driver1, 200, 100);

    // stepper1.step_for(1000);
    stepper1.revolve(-5);


    while (true) {

        stepper1.step();
        // stepper_driver1.step_pulse();      

    }

    return 0;
}
