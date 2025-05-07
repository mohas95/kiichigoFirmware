#ifndef StepperMotor_H
#define StepperMotor_H

#include "StepperDriver.h"
#include <array>


class StepperMotor {
    public:
        static constexpr std::array<uint8_t, 8> STEP_MODE_MULTIPLIER = {{
            1,  // FULL
            2,   // HALF
            4,   // QUARTER
            8,    // ONE_8
            16,   // ONE_16
            32,    // ONE_32
            64,    // ONE_64
            128      // ONE_128
        }};

        StepperMotor(const char* label,
                     StepperDriver& driver,
                     uint32_t steps_per_rev,
                     uint32_t default_speed=200); //in rpms

        void revolve(int32_t revolutions=1);
        void set_speed(uint32_t rpm=200);
        void home();
        void update_position();
        bool step();
        bool active();

    private:
        const char* label_;
        uint32_t steps_per_rev_, speed_;
        StepperDriver& driver_;
        int32_t position_step_;
        double position_revolutions_;

};


#endif