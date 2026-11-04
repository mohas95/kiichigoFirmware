# kiichigoFirmware

### Project Description
The goal of this project is to make a stepper motor motion firmware for RP2040/RP2350-based microcontroller. This will include: 
- a user-friendly library for abstracting stepper motor control for major stepper motor drivers.
- Simple motion planning for synchronous multi-axis machines such as CNC or 3D Printer
- Compatibility with G-code (in progress, not yet implemented)
- includes auxiliary functionality for limit switches

## Installation

Build from source
```bash
git clone git@github.com:mohas95/kiichigoFirmware.git
cd kiichigoFirmware
mkdir build
cd build
cmake ..
make
```

Then copy binary file (kiichigoFirmware.uf2) in the build folder to the raspberry pi pico

```bash
# kiichigoFirmware/build/kiichigoFirmware.uf2 --> Rpi Pico upload directory 
```

## Example usage

### motor library usage

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

    TB67S128FTG stepper_driver1(0, 1, 2, 3, 4, 5, StepperDriver::StepMode::HALF); //handles driver specific primatives
    StepperMotor stepper1( "x-axis", // const char* label
                            stepper_driver1, //StepperDriver& driver
                            200, //uint32_t steps_per_rev
                            100 //uint32_t default_speed=100 rpm
                         );

    //non-blocking revolutions
    stepper1.revolve( -5, // float revolutions (+-) indicates direction
                       100 // float speed rpm); 

    while (stepper1.active()) {
         printf("steps remaining:%i, step position:%i, rev position:%0.2f, deg position:%0.2f, angular position:%0.2f\n", 
                 stepper1.get_steps_remaining(), // steps remaining in revolve action 
                 stepper1.get_position_step(), // cumulative step position from instantiation of stepper object
                 stepper1.get_position_rev(), // cumulative position in terms of revolutions from instantiation
                 stepper1.get_position_deg(), // cumulative angular postion in degrees (replace deg with rad for radians)
                 stepper1.get_position_deg_wrapped()); // absolute/wrapped angular postion in degrees (replace deg with rad for radians)
    }

    stepper1.step_for(100, //int32_t discrete steps
                      100); //float speed rpm

    while (stepper1.active()) {
        if (stepper1.get_steps_remaining() <= 10){
            stepper.stop()// stops motor and sets steps remaining to 0
        }
    }

    stepper1.set_standbyMode(true); // puts the motor in standbymode to perserve power


    printf("Done!\n");
    return 0;
}
```

Using the LimitSwitch Library for abstracted monitoring of limit switches
``` cpp
#include <stdio.h>
#include "pico/stdlib.h"
#include "LimitSwitch.h"
#include "MCP23S17.h"


// MCP23S17 GPIO_ext Pin configuration
static constexpr uint PIN_MISO = 16;
static constexpr uint PIN_CS   = 17;
static constexpr uint PIN_SCK  = 18;
static constexpr uint PIN_MOSI = 19;
static constexpr uint PIN_INT  = 20;  

int main()
{
    stdio_init_all();

    // Wait for USB serial to be connected
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    // Print a message to the USB serial
    printf("USB Serial connected!\n");

    LimitSwitch home_switch( "home", // std::string label 
                             18, // uint8_t pin
                             0, // double fixed_position
                             {"x", "y"}, //std::vector<std::string> map_to={}  used to map motor labels to limit switch for motionPlanner
                             1, //uint8_t reverse_value how many revolutions to rotate in the opposite direction when limit switch is hit
                             LimitSwitch::PullMode::PULL_UP // PullMode mode = PullMode::PULL_UP    options: PULL_UP, PULL_DOWN, EXTERNAL_UP, EXTERNAL_DOWN
                             );

    //limit switch also supports MCP23S17 (I/O) expander for limited gpio space on pico

    // ---- SPI0 init (Mode 0) ----
    spi_init(spi0, 1'000'000);
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    // MCP23S17 Initialization
    //CS pin
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    //interrupt pin
    gpio_init(PIN_INT);
    gpio_pull_up(PIN_INT);
    gpio_set_dir(PIN_INT, GPIO_IN);
    //MCP23S17 Configuration
    MCP23S17 mcp(spi0, PIN_CS, /*hw_addr*/0);
    mcp.setInterruptOpenDrain(true); // INT as open-drain
    mcp.setInterruptPolarity(false); // active-LOW
    mcp.mirrorInterrupts(false);     // use INTA only
    mcp.interruptEnableA(0b00000111); // enable on A0..A2
    mcp.interruptControlA(0x00);      // 0 = compare to PREVIOUS (fires on change)
    (void)mcp.readA();                // prime previous state & clear any pending
    
    LimitSwitch z_switch(&mcp,'a', 2, "z_limit", 0, {"z"}, 1, LimitSwitch::PullMode::PULL_UP);



    while (true)
    {
        bool state = home_switch.get_state(true); // true: windowed filtering (naive mitigate signal interference), false: raw output
        printf("%s\n", state ? "button Pressed" : "not pressed");
        sleep_ms(1);
    }
    

    printf("Done!\n");
    return 0;
}
```

Concurrent Multi-motor operation the MotionCoordinator package
``` cpp
#include <stdio.h>
#include "pico/stdlib.h"

#include "Log.h"
#include "TB67S128FTG.h"
#include "StepperMotor.h"
#include "MotionCoordinator.h"
#include "LimitSwitch.h"


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
    LimitSwitch home_switch("home", 18, 0, {"x"}, 1, LimitSwitch::PullMode::PULL_UP);
    StepperMotor stepper1("x", stepper_driver1, 200, 100);

    TB67S128FTG stepper_driver2(6, 7, 8, 9, 10, 11, StepperDriver::StepMode::QUARTER);
    StepperMotor stepper2("y", stepper_driver2, 200, 100);

    TB67S128FTG stepper_driver3(12, 13, 14, 15, 17, 16, StepperDriver::StepMode::QUARTER);
    StepperMotor stepper3("z", stepper_driver3, 200, 100);

    config.stepper_motors={&stepper1, &stepper2, &stepper3}; // declare all stepper motors to be controlled by the MotionPlanner object in the the MotionConfig struct;
    config.limit_switches = {&home_switch}; // declare all limit switches to be monitored by the MotionPlanner object in the the MotionConfig struct;

    // instantiate the MotionCoordinator
    MotionCoordinator stepper_controller( config, //MotionConfig config: struct for configuring the motors to be used in the MotionPlanner object
                                      250); //uint16_t log_rate: feedback output interval in milliseconds


    stepper_controller.loop_forever(); //continous action request operation through serial Monitor, using the FIFO principle (this is blocking)
    /*Serial Command List: 
    
        1. MOVE <motorlabel1>,<revolutions>,<`rpm`>  <motorlabel2>,<revolutions>,<`rpm`>  ...: This command sets the number of revolutions that each motor in the motion planner should make, accepts float (ex. "MOVE x,100,50 y,-100,50 z,10,100")
        
        2. STANDBY <`motorlabel1`>,<`true`> <`motorlabel2`>,<`false`> ... : This command sets the number of speed of each motor in the motion planner, accepts bool or 1/0 (ex. "STANDBY x,1 y,0 z,true")

        3. HIT <`motorlabel1`>,<`set_position`>,<'reverse_after_hit'> <`motorlabel2`>,<`set_position`><'reverse_after_hit'> ... : This (immediate interupt) command interrupts operations and stops stepper motors and reverses motor, meant for limit switch operation, accepts double for position and input. It also revolves # if revolution in the opposite direction of the motion of the motor so that it does not rest on any limit switch (HIT X,10.0,0 Y,200.5,1 Z,-30.0,2")
        
        4. STOP <`motorlabel1`> <`motorlabel2`> ... : This command interrupts operations and stops stepper motors, but does not change the position tracking, just provide label name(ex. "STOP x y z")
    */

    printf("Done!\n");

    return 0;
}

```

## MotionCoordinator Event loop logic

- Commands are given through serial communication or through interrupts (via limit switch)
- Each serial command is added to the event loop queue in a FIFO fashion. Serial and interrupt commands are requested throughout the event loop.
- When the event loop is not busy performing an action, it will perform the next action in the queue (i.e. move, standby). 
- Interrupt commands (i.e stop) will interrupt an action, which clears the queue of actions but will remain open for new commands.  Flagged interrupts (i.e. hit) will do the same and trigger an interrupt flag which will clear any cached commands in the action queue and will stop any requests for new serial commands. When flagged interrupt action is finished, new commands can be sent again.
- Feedback from the motors and limit switches are sent as output of the serial monitor at fixed intervals (set when instantiating the motionCoordinator class)


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
### 1.0.0 (stable release)
- new serial command scheme for motorCoordinator package:
    1. MOVE <motorlabel1>,<revolutions>,<`rpm`>  <motorlabel2>,<revolutions>,<`rpm`>  ...: This command sets the number of revolutions that each motor in the motion planner should make, accepts float (ex. "MOVE x,100,50 y,-100,50 z,10,100")
    2. STANDBY <`motorlabel1`>,<`true`> <`motorlabel2`>,<`false`> ... : This command sets the number of speed of each motor in the motion planner, accepts bool or 1/0 (ex. "STANDBY x,1 y,0 z,true")
    3. HIT <`motorlabel1`>,<`set_position`>,<'reverse_after_hit'> <`motorlabel2`>,<`set_position`><'reverse_after_hit'> ... : This (immediate interupt) command interrupts operations and stops stepper motors and reverses motor, meant for limit switch operation, accepts double for position and input. It also revolves # if revolution in the opposite direction of the motion of the motor so that it does not rest on any limit switch (HIT X,10.0,0 Y,200.5,1 Z,-30.0,2")
    4. STOP <`motorlabel1`> <`motorlabel2`> ... : This command interrupts operations and stops stepper motors, but does not change the position tracking, just provide label name(ex. "STOP x y z")

### 0.1.0 (Not released yet)
- tested and operational ready for first version deployment
- Overhauling timing process from naive hardware timing to interupt request(IRQ) and Interrupt Service Routines(ISR)

### 0.0.1 (Not released yet)
- Complete Overhaul of old project, reorganization of project structure.
- StepperDriver Parent class for blueprint of various stepperdrivers on market.
- [TB67S128FTG](https://www.pololu.com/product/2998) Motor Driver Support added inherets from StepperDriver.
- StepperMotor library added to abstract StepperDriver controls, includes motor specific functionalities such as revolution control, position tracking. Dependent on StepperDriver class. 
- Logging module added to simplify displaying various levels of messages on Serial Monitor.
- support for limit switches
    - Limit switch class integrates lowpass signal filtering for debounce for stray signals
- Multi-Motor Control with the MotionPlanner Package.
    - Current Functionality is to receive commands from Serial USB 
    - Concurrent operation with the loop_forever() method (blocking)
    - USB Serial Command list:
        1. MOVE <`motorlabel1`>,<`revolutions`> <`motorlabel2`><`revolutions`> ... : This command sets the number of revolutions that each motor in the motion planner should make, accepts double (ex. "MOVE x,100 y,-100 z,10")
        2. SPEED <`motorlabel1`>,<`rpm`> <`motorlabel2`><`rpm`> ... : This command sets the number of speed of each motor in the motion planner, accepts double (ex. "SPEED x,100.0 y,200.0 z,50.0")
        3. STANDBY <`motorlabel1`>,<`true`> <`motorlabel2`><`false`> ... : This command sets the number of speed of each motor in the motion planner, accepts bool or 1/0 (ex. "STANDBY x,1 y,0 z,true")
        4. POSITION (in progress)
        5. HIT <`motorlabel1`>,<`set_position`>,<'reverse_after_hit'> <`motorlabel2`>,<`set_position`>,<'reverse_after_hit'> ... : This (flagged interupt) command interrupts operations and stops stepper motors, and sets the position tracker, meant for limit switch operation, accepts double for position and input. It also revolves # if revolution in the opposite direction of the motion of the motor so that it does not rest on any limit switch (HIT X,10.0,0 Y,200.5,1 Z,-30.0,2")
        6. STOP <`motorlabel1`> <`motorlabel2`> ... : This command interrupts operations and stops stepper motors, but does not change the position tracking, just provide label name(ex. "STOP x y z")
    - Bugs to address: 
        - STOP commands after interrupt is hit
        - Reduction of any clogging of busy event loop

## Todo
- Polynomial motion coordinator to accept polynomial motion inputs for smooth motion. (for version 1.1.0)
- Off load step generation and timing to PIO state machinges rather than hardware timer on the CPU. (eventually)

## Useful commands
serial terminal and command line communication for testing on Raspberry pi
```bash
picocom -b 115200 /dev/ttyACM0
printf 'stop x y z  \n' > /dev/ttyACM0
```