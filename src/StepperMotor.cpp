#include "StepperMotor.h"

StepperMotor::StepperMotor (const char* label,
                            StepperDriver& driver,
                            uint8_t steps_per_rev) : 
                            label_(label),
                            driver_(driver),
                            steps_per_rev_(steps_per_rev)
                            {}

void StepperMotor::revolve(uint32_t revolutions){
    uint32_t steps = revolutions*steps_per_rev_;
    driver_.step_for(revolutions);
}