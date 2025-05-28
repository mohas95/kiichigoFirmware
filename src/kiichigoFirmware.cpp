#include <stdio.h>
#include "pico/stdlib.h"

#include "Log.h"
#include "TB67S128FTG.h"
#include "StepperMotor.h"
#include "MotionPlanner.h"
#include "LimitSwitch.h"


MotionConfig config;

int main()
{
    stdio_init_all();

    log_set_level(LogLevel::INFO);


    // Wait for USB serial to be connected
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    // Print a message to the USB serial
    printf("USB Serial connected!\n");

    TB67S128FTG stepper_driver1(0, 1, 2, 3, 4, 5, StepperDriver::StepMode::HALF);
    LimitSwitch home_switch("home", 18, 0, {"x", "y"}, LimitSwitch::PullMode::PULL_UP);
    StepperMotor stepper1("x", stepper_driver1, 200, 100);

    TB67S128FTG stepper_driver2(6, 7, 8, 9, 10, 11, StepperDriver::StepMode::QUARTER);
    StepperMotor stepper2("y", stepper_driver2, 200, 100);

    config.stepper_motors={&stepper1, &stepper2};
    config.limit_switches = {&home_switch};

    // while (true)
    // {
    //     bool state = home_switch.get_state();
    //     printf("%s\n", state ? "button Pressed" : "not pressed");
    //     sleep_ms(1);
    // }
    

    MotionPlanner stepper_controller(config);


    stepper_controller.loop_forever(); //this is blocking

    printf("Done!\n");

    return 0;
}
