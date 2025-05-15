#include <stdio.h>
#include "pico/stdlib.h"

#include "Log.h"
#include "TB67S128FTG.h"
#include "StepperMotor.h"
#include "MotionPlanner.h"

MotionConfig config;

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
    StepperMotor stepper1("x-axis", stepper_driver1, 200, 100);

    config.stepper_motors={&stepper1};



    MotionPlanner stepper_controller(config);


    stepper_controller.loop_forever(); //this is blocking



    // stepper1.revolve(-5);

    // while (stepper1.active()) {
    //     stepper1.step();
    // }

    // stepper1.revolve(-5);

    // while (stepper1.active()) {
    //     stepper1.step();
    // }

    // stepper1.home();
    // stepper1.set_speed(200);
    // stepper1.revolve(10);

    // while (stepper1.active()) {
    //     stepper1.step();
    // }

    // stepper1.set_standbyMode(true);

    // while (true){
    // }

    printf("Done!\n");

    return 0;
}
