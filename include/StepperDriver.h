#ifndef StepperDriver_H
#define StepperDriver_H

#include <cstdint>
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


        // V0.1.0

        virtual void step_high() = 0;
        virtual void step_low() = 0;
        virtual void set_stepMode(StepMode) = 0;
        virtual StepMode get_stepMode() = 0;

        virtual void set_standbyMode(bool) = 0;
        virtual bool get_standbyMode() = 0;

        virtual void set_direction(bool) = 0;
        virtual bool get_direction() const =0;

        virtual void set_pulse_width(uint8_t) = 0;
        virtual uint8_t get_pulse_width() =0;


        
        virtual ~StepperDriver(){};

    private:



};


#endif