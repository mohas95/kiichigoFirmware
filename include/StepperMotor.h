#ifndef StepperMotor_H
#define StepperMotor_H

#include "StepperDriver.h"

class StepperMotor {
    public:
        StepperMotor(const char* label,
                     StepperDriver& driver,
                     uint8_t steps_per_rev);

        void revolve(uint32_t revolutions=1);



    private:
    const char*  label_;
    uint8_t steps_per_rev_;
    StepperDriver& driver_;
};


#endif