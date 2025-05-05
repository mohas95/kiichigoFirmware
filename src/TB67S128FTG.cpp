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
                          uint32_t pulse_width) 
                          : dirPin_(dirPin),
                            stepPin_(stepPin),
                            stbyPin_(stbyPin),
                            mode0Pin_(mode0Pin),
                            mode1Pin_(mode1Pin),
                            mode2Pin_(mode2Pin),
                            min_pulse_width_(pulse_width)
                            {

    // Initialize Pins and set to output 
    gpio_init(dirPin_); gpio_set_dir(dirPin_, GPIO_OUT);
    gpio_init(stepPin_); gpio_set_dir(stepPin_, GPIO_OUT);
    gpio_init(stbyPin_); gpio_set_dir(stbyPin_, GPIO_OUT);
    gpio_init(mode0Pin_); gpio_set_dir(mode0Pin_, GPIO_OUT);
    gpio_init(mode1Pin_); gpio_set_dir(mode1Pin_, GPIO_OUT);
    gpio_init(mode2Pin_); gpio_set_dir(mode2Pin_, GPIO_OUT);

    // Initialize values;
    start_time_us_ = 0;
    end_time_us_ = 0;
    
    // Set default states
    set_stepMode(step_mode);
    set_standbyMode(false);
    set_direction(true);  

}

void TB67S128FTG::set_standbyMode(bool active){
    if(stby_state_!=active){
        gpio_put(stbyPin_, !active);
        stby_state_ = active;
    }

    LOG_INFO("standby mode is: %s", active ? "Enabled" : "Disabled");
}

void TB67S128FTG::set_stepMode(StepMode step_mode){
    
    auto modeBits = STEP_MODE_BITS[static_cast<size_t>(step_mode)];
    gpio_put(mode0Pin_, modeBits[0]);
    gpio_put(mode1Pin_, modeBits[1]);
    gpio_put(mode2Pin_, modeBits[2]);

    current_stepMode_ = step_mode;
    LOG_INFO("Mode Pins set to 0: %s, 1: %s, 2: %s\n",
         modeBits[0] ? "true" : "false",
         modeBits[1] ? "true" : "false",
         modeBits[2] ? "true" : "false");
}

void TB67S128FTG::set_direction(bool direction){    
   
    if(dir_state_!=direction){
        gpio_put(dirPin_, direction);
        dir_state_ = direction;
    }

    LOG_INFO("direction set to: %s", direction ? "CW" : "CCW");
}

void TB67S128FTG::pulse_high(){

    uint64_t time_now = time_us_64(); 
    uint64_t time_diff = time_now-end_time_us_;

    if( !pulse_state_ && time_diff>=min_pulse_width_){
        gpio_put(stepPin_, true);
        start_time_us_ = time_now;
        pulse_state_=true;
    }
    
}

void TB67S128FTG::pulse_low(){
    uint64_t time_now = time_us_64(); 
    uint64_t time_diff = time_now-start_time_us_;

    if( pulse_state_ && time_diff>=min_pulse_width_){
        gpio_put(stepPin_, false);
        end_time_us_=time_now;
        pulse_state_=false;
    }
    
}