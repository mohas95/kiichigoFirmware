#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"

#include "Log.h"
#include "TB67S128FTG.h"
#include "StepperMotor.h"
#include "MotionCoordinator.h"
#include "LimitSwitch.h"
#include "MCP23S17.h"

//Fan Mosfet Pin
static constexpr uint PIN_FAN = 22;
const uint FAN_slice_num = pwm_gpio_to_slice_num(PIN_FAN);
const uint FAN_channel = pwm_gpio_to_channel(PIN_FAN);

// MCP23S17 GPIO_ext Pin configuration
static constexpr uint PIN_MISO = 16;
static constexpr uint PIN_CS   = 17;
static constexpr uint PIN_SCK  = 18;
static constexpr uint PIN_MOSI = 19;
static constexpr uint PIN_INT  = 20;   // INTA -> GPIO20

MotionConfig config;


int main(){
    stdio_init_all();
    log_set_level(LogLevel::DEBUG);

    //FAN Mosfet configuration
    gpio_set_function(PIN_FAN, GPIO_FUNC_PWM);
    pwm_set_wrap(FAN_slice_num, 1000);
    pwm_set_clkdiv(FAN_slice_num,4.0f);
    
    pwm_set_chan_level(FAN_slice_num, FAN_channel, 1000);
    pwm_set_enabled(FAN_slice_num, true);

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
    //interrupt pun
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

    // Wait for USB serial to be connected
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    // Print a message to the USB serial
    printf("USB Serial connected!\n");

    TB67S128FTG stepper_driver1(0, 1, 2, 3, 4, 5, StepperDriver::StepMode::QUARTER);
    // LimitSwitch home_switch("home", 18, 0, {"z"}, 1, LimitSwitch::PullMode::PULL_UP);
    LimitSwitch z_switch(&mcp,'a', 2, "z_limit", 0, {"z"}, 1, LimitSwitch::PullMode::PULL_UP);
    StepperMotor stepper1("z", stepper_driver1, 200, 100);

    TB67S128FTG stepper_driver2(6, 7, 8, 9, 10, 11, StepperDriver::StepMode::QUARTER);
    LimitSwitch y_switch(&mcp,'a', 1, "y_limit", 0, {"y"}, 0.5, LimitSwitch::PullMode::PULL_UP);
    StepperMotor stepper2("y", stepper_driver2, 200, 100);

    TB67S128FTG stepper_driver3(12, 13, 14, 15, 26, 27, StepperDriver::StepMode::QUARTER);
    LimitSwitch x_switch(&mcp,'a', 0, "x_limit", 0, {"x"}, 0.5, LimitSwitch::PullMode::PULL_UP);
    StepperMotor stepper3("x", stepper_driver3, 200, 100);


    config.stepper_motors={&stepper1, &stepper2, &stepper3};
    config.limit_switches = {&x_switch, &y_switch, &z_switch};
    
    MotionCoordinator stepper_controller(config,250);

    stepper_controller.loop_forever(); //this is blocking

    printf("Done!\n");

    return 0;

    // // Print a message to the USB serial
    // printf("USB Serial connected!\n");

    // TB67S128FTG stepper_driver1(6, 7, 8, 9, 10, 11, StepperDriver::StepMode::QUARTER);
    // // LimitSwitch home_switch("home", 18, 0, {"z"}, 1, LimitSwitch::PullMode::PULL_UP);
    // StepperMotor stepper1("z", stepper_driver1, 200, 50);

    // config.stepper_motors={&stepper1};

    // MotionCoordinator coordinator(config, 250);

    // coordinator.loop_forever(); //this is blocking

    // printf("Done!\n");

    // return 0;
}


    // stepper1.revolve(5,100);

    // while (stepper1.active()){

    //     printf("steps remaining:%i, step position:%i, rev position:%0.2f, deg position:%0.2f, angular position:%0.2f\n", 
    //         stepper1.get_steps_remaining(), stepper1.get_position_step(), stepper1.get_position_rev(), stepper1.get_position_deg(), stepper1.get_position_deg_wrapped());
    // }
    // printf("steps remaining:%i, step position:%i, rev position:%0.2f, deg position:%0.2f, angular position:%0.2f\n", 
    //     stepper1.get_steps_remaining(), stepper1.get_position_step(), stepper1.get_position_rev(), stepper1.get_position_deg(), stepper1.get_position_deg_wrapped());
    
    // sleep_ms(2000);
    
    // stepper1.revolve(-3,50);

    // while (stepper1.active()){

    //     printf("steps remaining:%i, step position:%i, rev position:%0.2f, deg position:%0.2f, angular position:%0.2f\n", 
    //         stepper1.get_steps_remaining(), stepper1.get_position_step(), stepper1.get_position_rev(), stepper1.get_position_deg(), stepper1.get_position_deg_wrapped());
    // }

    // printf("steps remaining:%i, step position:%i, rev position:%0.2f, deg position:%0.2f, angular position:%0.2f\n", 
    //     stepper1.get_steps_remaining(), stepper1.get_position_step(), stepper1.get_position_rev(), stepper1.get_position_deg(), stepper1.get_position_deg_wrapped());
    
    // sleep_ms(3000);
    
//
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

//     stepper1.set_standbyMode(true);


// }