#ifndef LimitSwitch_H
#define LimitSwitch_H

#include <cstdint>
#include <vector>
#include <string>

#include "pico/stdlib.h"



class LimitSwitch{
    public:

        enum class PullMode {
            EXTERNAL,       // No internal pull (external pull expected)
            PULL_UP,    // Internal pull-up
            PULL_DOWN   // Internal pull-down
        };
        
        LimitSwitch(std::string label, uint8_t pin, double fixed_pos, std::vector<std::string> map_to={}, PullMode mode = PullMode::EXTERNAL);

        bool get_state() const;
        uint8_t get_pin() const;
        double get_fixed_position() const;
        std::vector<std::string> get_mapping() const;
        const std::string& label() const;


    private:
        const double fixed_position_;
        uint8_t pin_;
        std::vector<std::string> mapping_;
        std::string label_;
};



#endif