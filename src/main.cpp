#include "TB67S128FTG.hpp"
#include "pico/stdlib.h"


int main(){

    stdio_init_all();
    
    TB67S128FTG stepper1(0, 1, 2, 3, 4, 5);

    while (true){

        stepper1.step_pulse();
        sleep_us(5000);
    }
    
};