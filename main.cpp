#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
#include <tuple>

using namespace std;

bool FULL_STEP[3] = {false, false, false};
bool HALF_STEP[3] = {false, false, true};
bool QUARTER_STEP[3] = {false, true, false};
bool ONE_8_STEP[3] = {false, true, true};
bool ONE_16_STEP[3] = {true, false, false};
bool ONE_32_STEP[3] = {true, false, true};
bool ONE_64_STEP[3] = {true, true, false};
bool ONE_128_STEP[3] = {true, true, true};


class TB67S128FTG{

    public:
        TB67S128FTG(unsigned int dirPin, unsigned int stepPin, unsigned int stbyPin, unsigned int mode0Pin, unsigned int mode1Pin, unsigned int mode2Pin)
        : dirPin(dirPin), stepPin(stepPin), stbyPin(stbyPin), mode0Pin(mode0Pin), mode1Pin(mode1Pin), mode2Pin(mode2Pin){

            // initialize pins
            gpio_init(dirPin);
            gpio_init(stepPin);
            gpio_init(stbyPin);
            gpio_init(mode0Pin);
            gpio_init(mode1Pin);
            gpio_init(mode2Pin);

            gpio_set_dir(dirPin, GPIO_OUT);
            gpio_set_dir(stepPin, GPIO_OUT);
            gpio_set_dir(stbyPin, GPIO_OUT);
            gpio_set_dir(mode0Pin, GPIO_OUT);
            gpio_set_dir(mode1Pin, GPIO_OUT);
            gpio_set_dir(mode2Pin, GPIO_OUT);

            // default states
            set_standby_mode(false);
            set_step_mode(FULL_STEP);
            set_direction(true); // Set default direction

        }

        void set_standby_mode(bool active = false){

            stby_state = active;
            gpio_put(stbyPin, !stby_state);

        }

        void set_step_mode(bool step_mode[3] = FULL_STEP){

            mode0_state=step_mode[0], mode1_state=step_mode[1], mode2_state=step_mode[2];

            gpio_put(mode0Pin, mode0_state);
            gpio_put(mode1Pin, mode1_state);
            gpio_put(mode2Pin, mode2_state);

        }

        void step_pulse(bool direction = dir_state){

            if (direction != dir_state){
                set_direction(direction);
            }

            gpio_put(stepPin, true);
            sleep_us(1);
            gpio_put(stepPin, false)
            // sleep_us(step_delay);
        }

        void set_direction(bool direction){

            dir_state = direction;
            gpio_put(dirPin, dir_state);
            sleep_us()
        }



    private:
        unsigned int dirPin, stepPin, stbyPin, mode0Pin, mode1Pin, mode2Pin;
        // unsigned long step_delay = 1000; // default delay time between steps in microseconds
        bool stby_state, mode0_state, mode1_state, mode2_state, dir_state;


};

int main(){

    stdio_init_all();
    
    TB67S128FTG stepper1(0, 1, 2, 3, 4, 5);

    while (true){

        stepper1.step_pulse();
        sleep_us(1000000);

    }
    
};