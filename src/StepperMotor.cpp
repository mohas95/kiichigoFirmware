#include "StepperMotor.h"
#include "Log.h"
#include <cmath>


StepperMotor::StepperMotor (const char* label,
                            StepperDriver &driver,
                            uint32_t steps_per_rev,
                            uint32_t default_speed) : 
                            label_(label),
                            driver_(driver),
                            steps_per_rev_(steps_per_rev)
                            {
//Set default values 
set_speed(default_speed);
}

void StepperMotor::revolve(int32_t revolutions){
    
    bool direction = revolutions>=0 ? true:false;

    driver_.set_direction(direction);
    
    uint32_t steps = std::abs(revolutions)*steps_per_rev_;
    driver_.step_for(steps);
}

void StepperMotor::set_speed(uint32_t rpm){
    speed_ = rpm;

    uint8_t pulse_width = driver_.get_pulse_width();
    double pulse_length = 60'000'000.0/(rpm*steps_per_rev_);
    uint32_t pulse_interval = static_cast<uint32_t>(std::round(pulse_length)) - pulse_width;

    driver_.set_pulse_interval(pulse_interval);

    LOG_DEBUG("%s Speed set to: %d rpm (%d us pulse inteval)\n", label_, rpm, pulse_interval);   

}