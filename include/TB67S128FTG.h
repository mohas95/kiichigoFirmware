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
                    StepMode step_mode = StepMode::FULL
                    );
        
        void set_standbyMode(bool active) override;
        void set_stepMode(StepMode step_mode) override;
        void set_direction(bool direction) override;
        void step_pulse() override;



        


    private:
        uint8_t dirPin_, stepPin_, stbyPin_, mode0Pin_, mode1Pin_, mode2Pin_;
        bool pulse_state_, stby_state_, dir_state_;
        uint64_t start_time_us_, end_time_us_;
        uint32_t min_pulse_width_; 
        StepMode current_stepMode_;

        void start_pulse() override;
        void update_pulse() override;

};



#endif