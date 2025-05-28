#include "LimitSwitch.h"

LimitSwitch::LimitSwitch( std::string label,
                          uint8_t pin,
                          double fixed_pos,
                          std::vector<std::string> map_to,
                          PullMode mode ) : label_(label),
                                            fixed_position_(fixed_pos),
                                            pin_(pin),
                                            mapping_(map_to){

    gpio_init(pin_); gpio_set_dir(pin_, GPIO_IN);gpio_disable_pulls(pin_);
        
    switch (mode) {
        case PullMode::PULL_UP:
            gpio_pull_up(pin_);
            break;
        case PullMode::PULL_DOWN:
            gpio_pull_down(pin_);
            break;
        case PullMode::EXTERNAL:
            break;
    }

}

bool LimitSwitch::get_state() const{
    
    return gpio_get(pin_);
}

uint8_t LimitSwitch::get_pin() const{
    return pin_;
}

doube LimitSwitch::get_fixed_position() const{
    return fixed_position_; 
}

std::vector<std::string> LimitSwitchL::get_mapping() const{
    return mapping_;
}

const std::string& label() const{
    return label_;
}