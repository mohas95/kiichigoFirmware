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


        void register_step();


        //Depreciated

        void revolve(double revolutions=1.0);
        void home();
        std::tuple<int32_t, double>  update_position();
        void update_position(double);
        bool step();

        
        int32_t get_position_step() const;
        double get_position_rev() const;
        bool get_standbyMode() const;
        const std::string& label() const;
        bool active();


    private:
        
        // v0.1.0
        static bool step_irq_timer_cb(repeating_timer_t* timer);
        static int64_t pulse_low_cb(alarm_id_t id, void* user_data);
        
        std::string label_;
        uint32_t steps_per_rev_;
        StepperDriver& driver_;

        repeating_timer_t timer_;
        volatile uint32_t steps_remaining_=0;
        double speed_rpm_, default_speed_rpm_; // in rpm
        uint32_t step_period_; //in us

        volatile int32_t position_step_= 0;

        // older need to sort
        double position_revolutions_= 0;



};


#endif