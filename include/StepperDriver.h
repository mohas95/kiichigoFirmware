#ifndef StepperDriver_H
#define StepperDriver_H

#include "pico/stdlib.h"

class StepperDriver {
    public:
        enum class StepMode : uint8_t {
            FULL,
            HALF,
            QUARTER,
            ONE_8,
            ONE_16,
            ONE_32,
            ONE_64,
            ONE_128
        };

        virtual void step_pulse() = 0;
        virtual void set_pulse_interval(uint32_t) = 0;
        virtual void set_pulse_width(uint8_t) = 0;
        virtual void step_for(uint32_t) = 0;


        // virtual void pulse_high() = 0;
        // virtual void pulse_low() = 0;

        virtual void set_stepMode(StepMode) = 0;
        virtual void set_standbyMode(bool) = 0;
        virtual void set_direction(bool) = 0;
        // virtual void standby_mode(bool) = 0;
        virtual ~StepperDriver(){};

    private:



};


#endif