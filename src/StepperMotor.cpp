#include "StepperMotor.h"
#include "Log.h"
#include <cmath>


StepperMotor::StepperMotor (const char* label,
                            StepperDriver &driver,
                            uint32_t steps_per_rev,
                            uint32_t default_speed) : 
                            label_(label),
                            driver_(driver)
                            {
    //Set default values 

    StepperDriver::StepMode step_mode = driver_.get_stepMode();
    auto modeMultiplier = STEP_MODE_MULTIPLIER[static_cast<size_t>(step_mode)];
    steps_per_rev_ = steps_per_rev*modeMultiplier;

    set_speed(default_speed);
    update_position();

    LOG_DEBUG("%s Defaults set: StepMode Multiplier- %d, steps per revolution- %d, \n", label_, modeMultiplier, steps_per_rev_);


}

void StepperMotor::revolve(int32_t revolutions){
    
    bool direction = revolutions>=0 ? true:false;
    driver_.set_direction(direction);
    
    LOG_DEBUG("%s direction set to: %s\n", label_, direction ? "CW" : "CCW");
    
    uint32_t steps = std::abs(revolutions)*steps_per_rev_;
    driver_.step_for(steps);
    
    LOG_DEBUG("%s set for: %d revolutions \n", label_, std::abs(revolutions));

}

void StepperMotor::set_speed(uint32_t rpm){
    speed_ = rpm;

    uint8_t pulse_width = driver_.get_pulse_width();
    double pulse_length = 60'000'000.0/(rpm*steps_per_rev_);
    uint32_t pulse_interval = static_cast<uint32_t>(std::round(pulse_length)) - pulse_width;

    driver_.set_pulse_interval(pulse_interval);

    LOG_DEBUG("%s Speed set to: %d rpm(%d us pulse inteval)\n", label_, rpm, pulse_interval);   

}

void StepperMotor::update_position(){
    position_step_ = driver_.get_step_tracker();
    position_revolutions_ = static_cast<double>(position_step_)/static_cast<double>(steps_per_rev_);

    LOG_DEBUG("%s Position: %d steps (%.2f revolutions) \n", label_, position_step_, position_revolutions_);   
}


void StepperMotor::step(){
    bool pulse_flag = driver_.step_pulse();
    
    if (pulse_flag){
        update_position();
    }
}