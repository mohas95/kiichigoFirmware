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

    TB67S128FTG stepper_driver1(6, 7, 8, 9, 10, 11, StepperDriver::StepMode::FULL);
    // LimitSwitch home_switch("home", 18, 0, {"z"}, 1, LimitSwitch::PullMode::PULL_UP);
    StepperMotor stepper1("z", stepper_driver1, 200, 50);

    // stepper1.step_for(1000, 50);

    // uint32_t counter = 0;

    // while (counter <= 2000){
    //      printf("%u\n", counter);
    //      counter++;
    //     sleep_ms(100);

    // }

    while (true) {
        stepper_driver1.step_high();
        sleep_us(50);

        stepper_driver1.step_low();
        sleep_ms(50);
    }
}