#ifndef LimitSwitch_H
#define LimitSwitch_H

#include <cstdint>
#include <vector>
#include <string>
#include <array>

#include "pico/stdlib.h"
#include "MCP23S17.h"



class LimitSwitch{
    public:
        static constexpr size_t FILTER_WINDOW_SIZE = 10; 
        static constexpr size_t FILTER_WINDOW_THRESH = 8;

        enum class PullMode {
            EXTERNAL_UP,       // external pull up
            EXTERNAL_DOWN,       // external pull down
            PULL_UP,    // Internal pull-up
            PULL_DOWN   // Internal pull-down
        };
        
        LimitSwitch(std::string label, uint8_t pin, double fixed_pos, std::vector<std::string> map_to={}, double reverse_value=0, PullMode mode = PullMode::PULL_UP);
        LimitSwitch(MCP23S17* gpio_ext, char port, uint8_t pin, std::string label, double fixed_pos, std::vector<std::string> map_to={}, double reverse_value=0, PullMode mode = PullMode::PULL_UP);

        bool get_state(bool with_filtering); //changes made for the low pass (delete this comment when works)
        uint8_t get_pin() const;
        double get_reverse_value() const;
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
        double reverse_value_;


        //added lowpass filtering integreation (delete this when it works)
        // Majority window vote (requires continuos fast requests)

        std::array<uint8_t, FILTER_WINDOW_SIZE > state_window_;
        size_t state_window_idx_=0;
        size_t state_window_sum_ = 0; 

        //if using MCP23S17 gpio extender for

        MCP23S17* gpio_ext_= nullptr;
        char port_;
};



#endif