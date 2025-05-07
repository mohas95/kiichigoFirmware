#include "TB67S128FTG.h"
#include "Log.h"
#include "pico/time.h"

TB67S128FTG::TB67S128FTG (uint8_t dirPin,
                          uint8_t stepPin,
                          uint8_t stbyPin,
                          uint8_t mode0Pin,
                          uint8_t mode1Pin,
                          uint8_t mode2Pin,
                          StepMode step_mode,
                          uint8_t default_pulse_width,
                          uint32_t default_pulse_interval)
                          : dirPin_(dirPin),
                            stepPin_(stepPin),
                            stbyPin_(stbyPin),
                            mode0Pin_(mode0Pin),
                            mode1Pin_(mode1Pin),
                            mode2Pin_(mode2Pin),
                            pulse_width_(default_pulse_width),
                            pulse_interval_(default_pulse_interval)
                            {

    // Initialize Pins and set to output 
    gpio_init(dirPin_); gpio_set_dir(dirPin_, GPIO_OUT); gpio_put(dirPin_, true);
    gpio_init(stepPin_); gpio_set_dir(stepPin_, GPIO_OUT);
    gpio_init(stbyPin_); gpio_set_dir(stbyPin_, GPIO_OUT); gpio_put(stbyPin_, true);
    gpio_init(mode0Pin_); gpio_set_dir(mode0Pin_, GPIO_OUT);
    gpio_init(mode1Pin_); gpio_set_dir(mode1Pin_, GPIO_OUT);
    gpio_init(mode2Pin_); gpio_set_dir(mode2Pin_, GPIO_OUT);

    // Initialize values;
    last_time_update_us_ = time_us_64();

    
    // Set default states
    set_stepMode(step_mode);
    set_standbyMode(false);
    set_direction(true);

}

void TB67S128FTG::set_standbyMode(bool active){
    gpio_put(stbyPin_, !active);
    stby_state_ = active;

    // LOG_INFO("standby mode is: %s\n", active ? "Enabled" : "Disabled");
}

bool TB67S128FTG::get_standbyMode(){
    return stby_state_;
}

void TB67S128FTG::set_stepMode(StepMode step_mode){
    
    auto modeBits = STEP_MODE_BITS[static_cast<size_t>(step_mode)];
    gpio_put(mode0Pin_, modeBits[0]);
    gpio_put(mode1Pin_, modeBits[1]);
    gpio_put(mode2Pin_, modeBits[2]);

    current_stepMode_ = step_mode;
    // LOG_INFO("Mode Pins set to 0: %s, 1: %s, 2: %s\n",
    //      modeBits[0] ? "true" : "false",
    //      modeBits[1] ? "true" : "false",
    //      modeBits[2] ? "true" : "false");
}

StepperDriver::StepMode TB67S128FTG::get_stepMode(){
    return current_stepMode_;
}

void TB67S128FTG::set_direction(bool direction){    

    gpio_put(dirPin_, direction);
    dir_state_ = direction;

    // LOG_INFO("direction set to: %s\n", direction ? "CW" : "CCW");
}


void TB67S128FTG::set_pulse_interval(uint32_t pulse_interval){
    pulse_interval_=pulse_interval;
}

void TB67S128FTG::set_pulse_width(uint8_t pulse_width){
    pulse_width_=pulse_width;
}


uint32_t TB67S128FTG::get_pulse_interval(){
    return pulse_interval_;
}

uint8_t TB67S128FTG::get_pulse_width(){
    return pulse_width_;
}


void TB67S128FTG::step_for(uint32_t steps){
    steps_=steps;
}

void TB67S128FTG::step_pulse(){

    uint64_t time_now = time_us_64();
    uint64_t time_diff = time_now-last_time_update_us_;

    if(steps_>0){

        if (pulse_state_ && time_diff>=pulse_width_){
            pulse_state_ = false;
            gpio_put(stepPin_, false);
            last_time_update_us_ = time_now;
            steps_--;
            // LOG_DEBUG("Pulse Remaining: %d\n", steps_);
            // LOG_DEBUG("Pulse: %s | Remaining: %d\n", pulse_state_ ? "HIGH" : "LOW", steps_);

            
        }else if (!pulse_state_ && time_diff>=pulse_interval_){
            pulse_state_ = true;
            gpio_put(stepPin_, true);
            last_time_update_us_ = time_now;

        }
        
    }
   
}
