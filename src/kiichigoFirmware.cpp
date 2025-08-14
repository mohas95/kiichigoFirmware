#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "Log.h"
#include "TB67S128FTG.h"
#include "StepperMotor.h"
#include "MotionPlanner.h"
#include "LimitSwitch.h"
#include "MCP23S17.h"


// MCP23S17 GPIO_ext Pin configuration
static constexpr uint PIN_MISO = 16;
static constexpr uint PIN_CS   = 17;
static constexpr uint PIN_SCK  = 18;
static constexpr uint PIN_MOSI = 19;
static constexpr uint PIN_INT  = 20;   // INTA -> GPIO20

MotionConfig config;

int main()
{
    stdio_init_all();
    log_set_level(LogLevel::OUTPUT);

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
    //
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

    TB67S128FTG stepper_driver1(0, 1, 2, 3, 4, 5, StepperDriver::StepMode::HALF);
    // LimitSwitch home_switch("home", 18, 0, {"z"}, 1, LimitSwitch::PullMode::PULL_UP);
    LimitSwitch z_switch(&mcp,'a', 2, "z_limit", 0, {"z"}, 2, LimitSwitch::PullMode::PULL_UP);
    StepperMotor stepper1("z", stepper_driver1, 200, 100);

    TB67S128FTG stepper_driver2(6, 7, 8, 9, 10, 11, StepperDriver::StepMode::QUARTER);
    LimitSwitch y_switch(&mcp,'a', 1, "y_limit", 0, {"y"}, 2, LimitSwitch::PullMode::PULL_UP);
    StepperMotor stepper2("y", stepper_driver2, 200, 100);

    TB67S128FTG stepper_driver3(12, 13, 14, 15, 26, 27, StepperDriver::StepMode::QUARTER);
    LimitSwitch x_switch(&mcp,'a', 0, "x_limit", 0, {"x"}, 2, LimitSwitch::PullMode::PULL_UP);
    StepperMotor stepper3("x", stepper_driver3, 200, 100);


    config.stepper_motors={&stepper1, &stepper2, &stepper3};
    config.limit_switches = {&x_switch, &y_switch, &z_switch};
    
    MotionPlanner stepper_controller(config,250);

    stepper_controller.loop_forever(); //this is blocking

    printf("Done!\n");

    return 0;
}



// #include <cstdio>
// #include "pico/stdlib.h"
// #include "hardware/spi.h"
// #include "MCP23S17.h"

// // ---- Pico pins (SPI0 defaults) ----
// static constexpr uint PIN_MISO = 16;
// static constexpr uint PIN_CS   = 17;   // adjust if your CS differs
// static constexpr uint PIN_SCK  = 18;
// static constexpr uint PIN_MOSI = 19;
// static constexpr uint PIN_INT  = 20;   // INTA -> GPIO20

// // Globals used by IRQ
// static MCP23S17* g_mcp = nullptr;
// static volatile bool g_irq_fired = false;

// // IRQ: just set a flag; do SPI work in main loop
// void gpio_irq_cb(uint gpio, uint32_t events) {
//     if (gpio == PIN_INT) g_irq_fired = true;
// }

// static inline void print_hit_line(uint8_t porta) {
//     // Pull-ups on A0..A2 => active-LOW = HIT when bit == 0
//     auto hit = [&](int bit){ return ((porta >> bit) & 1u) == 0; };
//     printf("A2:%s  A1:%s  A0:%s  |  PORTA=0x%02X\n",
//            hit(2) ? "HIT" : "NOT",
//            hit(1) ? "HIT" : "NOT",
//            hit(0) ? "HIT" : "NOT",
//            porta);
// }

// int main() {
//     stdio_init_all();
//     // Wait for USB serial to be connected
//     while (!stdio_usb_connected()) {
//         sleep_ms(100);
//     }


//     // ---- SPI0 init (Mode 0) ----
//     spi_init(spi0, 1'000'000);
//     spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
//     gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
//     gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
//     gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

//     // CS pin
//     gpio_init(PIN_CS);
//     gpio_set_dir(PIN_CS, GPIO_OUT);
//     gpio_put(PIN_CS, 1);

//     // ---- MCP23S17 object (A2..A0 assumed 0) ----
//     MCP23S17 mcp(spi0, PIN_CS, /*hw_addr*/0);
//     g_mcp = &mcp;

//     // --- (1) Prove SPI works: read-modify-write IOCON (enable HAEN) ---
//     uint8_t iocon_before = mcp.readReg(MCP23S17::Reg::IOCON);
//     mcp.writeReg(MCP23S17::Reg::IOCON, uint8_t(iocon_before | 0x08)); // HAEN=1
//     uint8_t iocon_after  = mcp.readReg(MCP23S17::Reg::IOCON);
//     printf("IOCON before=0x%02X after=0x%02X (HAEN bit set)\n", iocon_before, iocon_after);

//     // --- (2) Configure Port A: inputs with pull-ups on A0..A2 ---
//     mcp.pinModeA(0xFF);              // all inputs
//     mcp.setPullupA(0b00000111);      // enable on A0..A2

//     // --- (3) Interrupts on BOTH edges (compare-to-previous) for A0..A2 ---
//     mcp.setInterruptOpenDrain(true); // INT as open-drain
//     mcp.setInterruptPolarity(false); // active-LOW
//     mcp.mirrorInterrupts(false);     // use INTA only

//     mcp.interruptEnableA(0b00000111); // enable on A0..A2
//     mcp.interruptControlA(0x00);      // 0 = compare to PREVIOUS (fires on change)
//     (void)mcp.readA();                // prime previous state & clear any pending

//     // Pico INT pin: input w/ pull-up, IRQ on falling edge (active-low INT)
//     gpio_init(PIN_INT);
//     gpio_pull_up(PIN_INT);
//     gpio_set_dir(PIN_INT, GPIO_IN);
//     gpio_set_irq_enabled_with_callback(PIN_INT, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_cb);

//     // Print initial state
//     uint8_t last_porta = mcp.readA();
//     printf("Ready. Initial: "); print_hit_line(last_porta);

//     // Poll timer (continuous status) ~10 Hz
//     absolute_time_t next_poll = make_timeout_time_ms(100);

//     while (true) {
//         // (A) Event-driven: handle any interrupt edges
//         if (g_irq_fired) {
//             g_irq_fired = false;

//             // Which pins caused it, and what was latched at the moment?
//             uint8_t flags = mcp.interruptFlags('A'); // which bits triggered
//             uint8_t latched = mcp.interruptCapture('A'); // read clears IRQ on A

//             printf("\n--- INTA --- flags=0x%02X  INTCAPA=0x%02X\n", flags, latched);
//             print_hit_line(latched);

//             last_porta = latched; // keep continuity with polling
//         }

//         // (B) Continuous: poll & print on change (every 100 ms)
//         if (time_reached(next_poll)) {
//             next_poll = make_timeout_time_ms(100);
//             uint8_t porta = mcp.readA();
//             if (porta != last_porta) {
//                 print_hit_line(porta);
//                 last_porta = porta;
//             }
//         }

//         tight_loop_contents();
//     }
//     return 0;
// }
