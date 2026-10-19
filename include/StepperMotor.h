#ifndef StepperMotor_H
#define StepperMotor_H

#include "StepperDriver.h"
#include <array>
#include <tuple>
#include <string>
#include "pico/time.h"


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
                     double default_speed_rpm=100); //in rpm

        
        // version 0.1.0
        
        void step_for(int32_t steps, double speed_rpm);
        uint32_t get_step_period() const;

        bool set_speed(double rpm=200);
        double get_speed() const;
        
        bool get_direction();
        void set_standbyMode(bool);
        bool get_standbyMode() const;

        void change_stepMode(StepperDriver::StepMode step_mode);
        StepperDriver::StepMode get_stepMode() const; 

        bool active() const;
        uint32_t get_steps_remaining() const;

        void revolve(double revolutions, double speed_rpm);
        
        int32_t get_position_step() const;
        double get_position_rev() const;
       
        double get_position_deg() const;
        double get_position_deg_wrapped() const;
        
        double get_position_rad() const;
        double get_position_rad_wrapped() const;

        const std::string& label() const;


        //Depreciated
        // TODO: Phase out and delete non useful members
        void home();
        std::tuple<int32_t, double>  update_position();
        void update_position(double);
        bool step();

    private:
        
        // v0.1.0
        static bool step_irq_timer_cb(repeating_timer_t* timer);
        static int64_t pulse_low_cb(alarm_id_t id, void* user_data);
        void register_step();
        void reset_steps_to_zero();
        
        std::string label_;
        uint32_t default_steps_per_rev_; //original full stepmode configuration of the motor
        uint32_t steps_per_rev_; //virtual step mode based on driver step mode
        StepperDriver& driver_;

        repeating_timer_t timer_;
        volatile uint32_t steps_remaining_=0;
        double speed_rpm_, default_speed_rpm_; // in rpm
        uint32_t step_period_; //in us

        volatile int32_t position_step_= 0;

        // older need to sort TODO: Phase out and delete non useful members
        double position_revolutions_= 0;



};


#endif