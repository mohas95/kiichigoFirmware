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

        virtual bool step_pulse() = 0;
        virtual void set_pulse_interval(uint32_t) = 0;
        virtual void set_pulse_width(uint8_t) = 0;
        virtual void step_for(uint32_t) = 0;

        virtual void set_stepMode(StepMode) = 0;
        virtual void set_standbyMode(bool) = 0;
        virtual void set_direction(bool) = 0;

        virtual uint32_t get_pulse_interval() =0;
        virtual uint8_t get_pulse_width() =0;
        virtual StepMode get_stepMode() = 0;
        virtual bool get_standbyMode() = 0;
        virtual bool get_direction() const =0;

        virtual void home() = 0;
        virtual int32_t get_step_tracker() = 0;
        virtual void set_step_tracker(int32_t) = 0;
        virtual bool active() = 0;

        
        virtual ~StepperDriver(){};

    private:



};


#endif