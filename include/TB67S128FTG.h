#ifndef TB67S128FTG_H
#define TB67S128FTG_H

#include "StepperDriver.h"
#include <array>
#include <cstdint>

class TB67S128FTG : public StepperDriver{
    public:

        static constexpr std::array<std::array<bool, 3>, 8> STEP_MODE_BITS = {{
            {false, false, false},  // FULL
            {false, false, true},   // HALF
            {false, true, false},   // QUARTER
            {false, true, true},    // ONE_8
            {true, false, false},   // ONE_16
            {true, false, true},    // ONE_32
            {true, true, false},    // ONE_64
            {true, true, true}      // ONE_128
        }};

        TB67S128FTG(uint8_t dirPin,
                    uint8_t stepPin,
                    uint8_t stbyPin,
                    uint8_t mode0Pin,
                    uint8_t mode1Pin,
                    uint8_t mode2Pin,
                    StepMode step_mode = StepMode::FULL,
                    uint8_t default_pulse_width=5,
                    uint32_t default_pulse_interval=500000
                    );
        
        void set_standbyMode(bool active) override;
        void set_stepMode(StepMode step_mode) override;
        void set_direction(bool direction) override;
        
        void set_pulse_interval(uint32_t pulse_interval) override;
        void set_pulse_width(uint8_t pulse_width) override;
        void step_for(uint32_t steps) override;
        
        uint32_t get_pulse_interval() override;
        uint8_t get_pulse_width() override;
        StepMode get_stepMode() override;
        bool get_standbyMode() override;
        bool get_direction() const override;


        void home() override;

        bool step_pulse() override;
        int32_t get_step_tracker() override;
        void set_step_tracker(int32_t) override;
        bool active() override;



    private:
        uint8_t dirPin_, stepPin_, stbyPin_, mode0Pin_, mode1Pin_, mode2Pin_;
        bool pulse_state_=false, stby_state_, dir_state_;
        uint64_t last_time_update_us_;
        uint8_t pulse_width_; // in microseconds
        uint32_t pulse_interval_; // in microseconds
        StepMode current_stepMode_;
        uint32_t steps_=0;
        int32_t step_tracker_;

};



#endif