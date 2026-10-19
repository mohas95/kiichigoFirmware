#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"

#include "Log.h"
#include "TB67S128FTG.h"
#include "StepperMotor.h"


int main(){
    stdio_init_all();
    log_set_level(LogLevel::DEBUG);

    // Wait for USB serial to be connected
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    // Print a message to the USB serial
    printf("USB Serial connected!\n");

    TB67S128FTG stepper_driver1(6, 7, 8, 9, 10, 11, StepperDriver::StepMode::QUARTER);
    // LimitSwitch home_switch("home", 18, 0, {"z"}, 1, LimitSwitch::PullMode::PULL_UP);
    StepperMotor stepper1("z", stepper_driver1, 200, 50);


    stepper1.revolve(5,100);

    while (stepper1.active()){

        printf("steps remaining:%i, step position:%i, rev position:%0.2f, deg position:%0.2f, angular position:%0.2f\n", 
            stepper1.get_steps_remaining(), stepper1.get_position_step(), stepper1.get_position_rev(), stepper1.get_position_deg(), stepper1.get_position_deg_wrapped());
    }
    printf("steps remaining:%i, step position:%i, rev position:%0.2f, deg position:%0.2f, angular position:%0.2f\n", 
        stepper1.get_steps_remaining(), stepper1.get_position_step(), stepper1.get_position_rev(), stepper1.get_position_deg(), stepper1.get_position_deg_wrapped());
    
    sleep_ms(2000);
    
    stepper1.revolve(-3,50);

    while (stepper1.active()){

        printf("steps remaining:%i, step position:%i, rev position:%0.2f, deg position:%0.2f, angular position:%0.2f\n", 
            stepper1.get_steps_remaining(), stepper1.get_position_step(), stepper1.get_position_rev(), stepper1.get_position_deg(), stepper1.get_position_deg_wrapped());
    }

    printf("steps remaining:%i, step position:%i, rev position:%0.2f, deg position:%0.2f, angular position:%0.2f\n", 
        stepper1.get_steps_remaining(), stepper1.get_position_step(), stepper1.get_position_rev(), stepper1.get_position_deg(), stepper1.get_position_deg_wrapped());
    
    sleep_ms(3000);
    

    // stepper1.step_for(1000, 100);

    // while (stepper1.active())
    // {

    //     printf("steps remaining:%i, position:%i\n", stepper1.get_steps_remaining(), stepper1.get_position_step());
    // }

    // stepper1.step_for(-500, 10);


    // while (stepper1.active())
    // {
    //     // stepper1.step_for(-1000, 100); // some racing happening, will need to debug in the future

    //     printf("steps remaining:%i, position:%i\n", stepper1.get_steps_remaining(), stepper1.get_position_step());
    // }

    // printf("steps remaining:%i, position:%i\n", stepper1.get_steps_remaining(), stepper1.get_position_step());


    // stepper1.change_stepMode(StepperDriver::StepMode::FULL);

    // // stepper1.set_standbyMode(true);

    // stepper1.step_for(1000, 200);


    // while (stepper1.active())
    // {
    //     printf("steps remaining:%i, position:%i\n", stepper1.get_steps_remaining(), stepper1.get_position_step());
    // }

    // printf("steps remaining:%i, position:%i\n", stepper1.get_steps_remaining(), stepper1.get_position_step());

    stepper1.set_standbyMode(true);


}