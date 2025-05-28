#ifndef StepperMotor_H
#define StepperMotor_H

#include "StepperDriver.h"
#include <array>
#include <tuple>
#include <string>




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

        StepperMotor(std::string label,
                     StepperDriver& driver,
                     uint32_t steps_per_rev,
                     double default_speed=200); //in rpm

        void revolve(double revolutions=1.0);
        void set_speed(double rpm=200);
        void home();
        std::tuple<int32_t, double>  update_position();
        void update_position(double);
        bool step();
        void set_standbyMode(bool);
        bool active();

        const std::string& label() const;

    private:
        std::string label_;
        uint32_t steps_per_rev_;
        StepperDriver& driver_;
        int32_t position_step_;
        double position_revolutions_, speed_;

};


#endif