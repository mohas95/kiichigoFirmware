# kiichigoFirmware

### Project Description
The goal of this project is to make a stepper motor motion firmware for RP2040/RP2350-based microcontroller. This will include: 
- a user-friendly library for abstracting stepper motor control for major stepper motor drivers.
- Simple motion planning for synchronous multi-axis machines such as CNC or 3D Printer
- Compatibility with G-code

## Example usage
### motor library usage

Controlling directly from motor driver
``` cpp
#include <stdio.h>
#include "pico/stdlib.h"
#include "TB67S128FTG.h"


int main()
{
    stdio_init_all();

    // Wait for USB serial to be connected
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    // Print a message to the USB serial
    printf("USB Serial connected!\n");

    TB67S128FTG stepper_driver1(0, 
                                1, 
                                2, 
                                3, 
                                4, 
                                5, 
                                StepperDriver::StepMode::HALF);

    stepper_driver1.set_pulse_interval(5000); // sets pulse intervals (speed) in microseconds
    stepper_driver1.step_for(1000); // sets number of steps to take

    while (true) {
        stepper_driver1.step_pulse(); //runs pulse process in the event loop for parralelism
    }

    return 0;
}
```

Using the StepperMotor Library for abstracted control
``` cpp
#include <stdio.h>
#include "pico/stdlib.h"
#include "TB67S128FTG.h"
#include "StepperMotor.h"

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

    stepper1.revolve(-5); // five revolutions in the counter clockwise directions


    while (true) {

        stepper_driver1.step_pulse();      

    }

    return 0;
}
```


## Feedback
All kinds of feedback and contributions are welcome.
- [Create an issue](https://github.com/mohas95/kiichigoFirmware/issues)
- [Create a pull request](https://github.com/mohas95/kiichigoFirmware/pulls)
- reach out to @mohas95

## Contributors
- Mohamed Debbagh
    - [github](https://github.com/mohas05/), [website](https://mohas95.github.io/)

## Change Log
### 0.0.1 (Not released yet)
- Complete Overhaul of old project, reorganization of project structure.
- StepperDriver Parent class for blueprint of various stepperdrivers on market.
- [TB67S128FTG](https://www.pololu.com/product/2998) Motor Driver Support added inherets from StepperDriver.
- StepperMotor library added to abstract StepperDriver controls, includes motor specific functionalities such as revolution control. Dependent on StepperDriver class.
- Logging module added to simplify displaying various levels of messages on Serial Monitor.