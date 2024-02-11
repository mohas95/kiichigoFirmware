#ifndef StepperMotorDriver_HPP
#define StepperMotorDriver_HPP

#include "pico/stdlib.h"
#include <array>

class StepperMotorDriver{
    public:
        virtual void step_pulse() = 0;
        virtual void step_pulse(bool direction){}
        virtual void set_direction(bool direction) = 0;
        virtual bool get_direction() = 0;

};

class TB67S128FTG : public StepperMotorDriver{
    public:
        TB67S128FTG(unsigned int dirPin, unsigned int stepPin, unsigned int stbyPin, unsigned int mode0Pin, unsigned int mode1Pin, unsigned int mode2Pin)
        : dirPin(dirPin), stepPin(stepPin), stbyPin(stbyPin), mode0Pin(mode0Pin), mode1Pin(mode1Pin), mode2Pin(mode2Pin){
            // initialize pins
            gpio_init(dirPin);
            gpio_init(stepPin);
            gpio_init(stbyPin);
            gpio_init(mode0Pin);
            gpio_init(mode1Pin);
            gpio_init(mode2Pin);

            gpio_set_dir(dirPin, GPIO_OUT);
            gpio_set_dir(stepPin, GPIO_OUT);
            gpio_set_dir(stbyPin, GPIO_OUT);
            gpio_set_dir(mode0Pin, GPIO_OUT);
            gpio_set_dir(mode1Pin, GPIO_OUT);
            gpio_set_dir(mode2Pin, GPIO_OUT);

            // default states
            set_standby_mode(false);
            set_step_mode(FULL_STEP);
            set_direction(true); // Set default direction
        }

        // Step modes defined as public static constexpr arrays
        static constexpr std::array<bool, 3> FULL_STEP = {false, false, false};
        static constexpr std::array<bool, 3> HALF_STEP = {false, false, true};
        static constexpr std::array<bool, 3> QUARTER_STEP = {false, true, false};
        static constexpr std::array<bool, 3> ONE_8_STEP = {false, true, true};
        static constexpr std::array<bool, 3> ONE_16_STEP = {true, false, false};
        static constexpr std::array<bool, 3> ONE_32_STEP = {true, false, true};
        static constexpr std::array<bool, 3> ONE_64_STEP = {true, true, false};
        static constexpr std::array<bool, 3> ONE_128_STEP = {true, true, true};

        // StepperMotorDrive Parent pure virtual method overrides
        void step_pulse() override{
            gpio_put(stepPin, true);
            sleep_us(1);
            gpio_put(stepPin, false);
        }

        void step_pulse(bool direction) override{
            if (direction != dir_state){
                set_direction(direction);
            }
            step_pulse();
        }

        void set_direction(bool direction) override{
            dir_state = direction;
            gpio_put(dirPin, dir_state);
        }

        bool get_direction() override{
            return dir_state;
        }

        // Class Specific Methods
        void set_standby_mode(bool active = false){
            stby_state = active;
            gpio_put(stbyPin, !stby_state);
        }

        void set_step_mode(const std::array<bool, 3>& step_mode= FULL_STEP){
            mode0_state=step_mode[0], mode1_state=step_mode[1], mode2_state=step_mode[2];

            gpio_put(mode0Pin, mode0_state);
            gpio_put(mode1Pin, mode1_state);
            gpio_put(mode2Pin, mode2_state);
        }

        bool get_stby_state(){
            return stby_state; 
        }

        std::array<bool, 3> get_step_mode(){
            std::array<bool, 3> step_mode = {mode0_state, mode1_state, mode2_state};
            return step_mode;
        }

    private:
        unsigned int dirPin, stepPin, stbyPin, mode0Pin, mode1Pin, mode2Pin;
        bool stby_state, mode0_state, mode1_state, mode2_state, dir_state;
        
};




#endif