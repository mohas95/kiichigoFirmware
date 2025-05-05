#include <stdio.h>
#include "pico/stdlib.h"

#include "Log.h"
#include "TB67S128FTG.h"


void step(){


    
}


int main()
{
    stdio_init_all();

    TB67S128FTG md1(0, 1, 2, 3, 4, 5);

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }

    return 0;
}
