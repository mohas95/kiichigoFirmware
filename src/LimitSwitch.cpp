#include "LimitSwitch.h"

LimitSwitch::LimitSwitch( std::string label,
                          uint8_t pin,
                          double fixed_pos,
                          std::vector<std::string> map_to,
                          PullMode mode ) : label_(label),
                                            fixed_position_(fixed_pos),
                                            pin_(pin),
                                            pull_mode_(mode),
                                            mapping_(map_to){

    gpio_init(pin_); gpio_set_dir(pin_, GPIO_IN);gpio_disable_pulls(pin_);
        
    switch (pull_mode_) {
        case PullMode::PULL_UP:
            gpio_pull_up(pin_);
            break;
        case PullMode::PULL_DOWN:
            gpio_pull_down(pin_);
            break;
        case PullMode::EXTERNAL_UP:
            break;
        case PullMode::EXTERNAL_DOWN:
            break;
    }
    }

}

bool LimitSwitch::get_state() const{

    bool state = gpio_get(pin_);

    return is_active_low() ? !state : state;

}

uint8_t LimitSwitch::get_pin() const{
    return pin_;
}

double LimitSwitch::get_fixed_position() const{
    return fixed_position_; 
}

std::vector<std::string> LimitSwitch::get_mapping() const{
    return mapping_;
}

const std::string& LimitSwitch::label() const{
    return label_;
}


bool LimitSwitch::is_active_low() const {
    return pull_mode_ == PullMode::PULL_UP || pull_mode_ == PullMode::EXTERNAL_UP;
}