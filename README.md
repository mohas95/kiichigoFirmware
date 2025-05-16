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

    TB67S128FTG stepper_driver1(0, // uint8_t dirPin
                                1, // uint8_t stepPin
                                2, // uint8_t stbyPin
                                3, // uint8_t mode0Pin
                                4, // uint8_t mode1Pin
                                5, // uint8_t mode2Pin
                                StepperDriver::StepMode::HALF, //StepMode step_mode= StepMode::FULL,
                                5, //uint8_t default_pulse_width=5 microseconds
                                500000 //uint32_t default_pulse_interval=5000 in microseconds
                                ); 

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
    StepperMotor stepper1( "x-axis", // const char* label
                            stepper_driver1, //StepperDriver& driver
                            200, //uint32_t steps_per_rev
                            100 //uint32_t default_speed=200
                         );

    stepper1.revolve(-5);

    while (stepper1.active()) {
        stepper1.step();
    }

    stepper1.revolve(-5);

    while (stepper1.active()) {
        stepper1.step();
    }

    stepper1.home();
    stepper1.set_speed(200);
    stepper1.revolve(10);

    while (stepper1.active()) {
        stepper1.step();
    }

    stepper1.set_standbyMode(true);

    while (true){
    }

    printf("Done!\n");
    return 0;
}
```

Concurrent Multi-motor operation the MotionPlanner package
``` cpp
#include <stdio.h>
#include "pico/stdlib.h"

#include "Log.h"
#include "TB67S128FTG.h"
#include "StepperMotor.h"
#include "MotionPlanner.h"

MotionConfig config; //struct for configuring the motors to be used in the MotionPlanner object

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
    StepperMotor stepper1("x", stepper_driver1, 200, 100);

    config.stepper_motors={&stepper1}; // declare all stepper motors to be controlled by the MotionPlanner object in the the MotionConfig struct;

    MotionPlanner stepper_controller(config); // instantiate the MotionPlanner


    stepper_controller.loop_forever(); //continous action request operation through serial Monitor, using the FIFO principle (this is blocking)
    /*Serial Command List: 
    
        1. MOVE <motorlabel1>,<revolutions> <motorlabel2><revolutions> ...: This command sets the number of revolutions that each motor in the motion planner should make, currently only accepts int (ex. "MOVE x,100 y,-100 z,10")
        
        2. SPEED <`motorlabel1`>,<`rpm`> <`motorlabel2`><`rpm`> ... : This command sets the number of speed of each motor in the motion planner, accepts uint (ex. "SPEED x,100 y,200 z,50")
        
        3. STANDBY <`motorlabel1`>,<`true`> <`motorlabel2`><`false`> ... : This command sets the number of speed of each motor in the motion planner, accepts bool or 1/0 (ex. "STANDBY x,1 y,0 z,true")
    */

    printf("Done!\n");

    return 0;
}

```




## Feedback
All kinds of feedback and contributions are welcome.
- [Create an issue](https://github.com/mohas95/kiichigoFirmware/issues)
- [Create a pull request](https://github.com/mohas95/kiichigoFirmware/pulls)
- reach out to @mohas95

## Contributors
- Mohamed Debbagh (Main Author)
    - [github](https://github.com/mohas05/), [website](https://mohas95.github.io/)
- Axel Refalo
    - [github](https://github.com/axelrefalo)

## Change Log
### 0.0.1 (Not released yet)
- Complete Overhaul of old project, reorganization of project structure.
- StepperDriver Parent class for blueprint of various stepperdrivers on market.
- [TB67S128FTG](https://www.pololu.com/product/2998) Motor Driver Support added inherets from StepperDriver.
- StepperMotor library added to abstract StepperDriver controls, includes motor specific functionalities such as revolution control, position tracking. Dependent on StepperDriver class. 
- Logging module added to simplify displaying various levels of messages on Serial Monitor.
- Multi-Motor Control with the MotionPlanner Package.
    - Current Functionality is to receive commands from Serial USB 
    - Concurrent operation with the loop_forever() method (blocking)
    - USB Serial Command list:
        1. MOVE <`motorlabel1`>,<`revolutions`> <`motorlabel2`><`revolutions`> ... : This command sets the number of revolutions that each motor in the motion planner should make, accepts double (ex. "MOVE x,100 y,-100 z,10")
        2. SPEED <`motorlabel1`>,<`rpm`> <`motorlabel2`><`rpm`> ... : This command sets the number of speed of each motor in the motion planner, accepts uint (ex. "SPEED x,100 y,200 z,50")
        3. STANDBY <`motorlabel1`>,<`true`> <`motorlabel2`><`false`> ... : This command sets the number of speed of each motor in the motion planner, accepts bool or 1/0 (ex. "STANDBY x,1 y,0 z,true")
        4. POSITION (in progress)
        5. HOME (in progress)
