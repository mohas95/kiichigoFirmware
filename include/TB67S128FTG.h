#ifndef TB67S128FTG_H
#define TB67S128FTG_H

#include "StepperDriver.h"
#include <array>



class TB67S128FTG : public StepperDriver{
    public:
        // Step modes defined as public static constexpr arrays
        // static constexpr std::array<bool, 3> FULL_STEP = {false, false, false};
        // static constexpr std::array<bool, 3> HALF_STEP = {false, false, true};
        // static constexpr std::array<bool, 3> QUARTER_STEP = {false, true, false};
        // static constexpr std::array<bool, 3> ONE_8_STEP = {false, true, true};
        // static constexpr std::array<bool, 3> ONE_16_STEP = {true, false, false};
        // static constexpr std::array<bool, 3> ONE_32_STEP = {true, false, true};
        // static constexpr std::array<bool, 3> ONE_64_STEP = {true, true, false};
        // static constexpr std::array<bool, 3> ONE_128_STEP = {true, true, true};

        enum class StepMode : uint8_t {
            FULL,
            HALF,
            QUARTER,
            ONE_8,
            ONE_16,
            ONE_32,
            ONE_64,
            ONE_128
        };
        
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

        TB67S128FTG(unsigned int dirPin,
                    unsigned int stepPin,
                    unsigned int stbyPin,
                    unsigned int mode0Pin,
                    unsigned int mode1Pin,
                    unsigned int mode2Pin,
                    const std::array<bool,3>& step_mode = FULL_STEP
                    );
        


    private:
        unsigned int dirPin_, stepPin_, stbyPin_, mode0Pin_, mode1Pin_, mode2Pin_;
};



#endif