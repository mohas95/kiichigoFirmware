#ifndef StepperDriver_H
#define StepperDriver_H

#include "pico/stdlib.h"

class StepperDriver {
    public:
        virtual void step_pulse() = 0;
        virtual void set_direction() = 0;
        virtual void standby_mode(bool) = 0;
        virtual ~StepperDriver(){};

};


#endif