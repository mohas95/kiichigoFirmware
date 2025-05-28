#ifndef LimitSwitch_H
#define LimitSwitch_H

#include <cstdint>
#include <vector>
#include <string>

#include "pico/stdlib.h"



class LimitSwitch{
    public:

        enum class PullMode {
            EXTERNAL_UP,       // external pull up
            EXTERNAL_DOWN,       // external pull down
            PULL_UP,    // Internal pull-up
            PULL_DOWN   // Internal pull-down
        };
        
        LimitSwitch(std::string label, uint8_t pin, double fixed_pos, std::vector<std::string> map_to={}, PullMode mode = PullMode::PULL_UP);

        bool get_state() const;
        uint8_t get_pin() const;
        double get_fixed_position() const;
        std::vector<std::string> get_mapping() const;
        const std::string& label() const;

        bool is_active_low() const;



    private:
        const double fixed_position_;
        PullMode pull_mode_;
        uint8_t pin_;
        std::vector<std::string> mapping_;
        std::string label_;
};



#endif