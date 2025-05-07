#ifndef StepperMotor_H
#define StepperMotor_H

#include "StepperDriver.h"

class StepperMotor {
    public:
        StepperMotor(const char* label,
                     StepperDriver& driver,
                     uint32_t steps_per_rev,
                     uint32_t default_speed=200); //in rpms

        void revolve(int32_t revolutions=1);
        void set_speed(uint32_t rpm=200);

    private:
        const char*  label_;
        uint32_t steps_per_rev_;
        uint32_t speed_;
        StepperDriver& driver_;
};


#endif