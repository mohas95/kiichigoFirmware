#include "StepperMotor.h"
#include "Log.h"
#include <cmath>
#include "pico/time.h"

StepperMotor::StepperMotor (std::string label,
                            StepperDriver &driver,
                            uint32_t steps_per_rev,
                            double default_speed_rpm) : 
                            label_(label),
                            driver_(driver),
                            default_speed_rpm_(default_speed_rpm)
                            {
    //Set default values 

    StepperDriver::StepMode step_mode = driver_.get_stepMode();
    auto modeMultiplier = STEP_MODE_MULTIPLIER[static_cast<size_t>(step_mode)];
    default_steps_per_rev_ = steps_per_rev;
    steps_per_rev_ = default_steps_per_rev_ * modeMultiplier;

    set_speed(default_speed_rpm);
    // update_position(); // depreciated
    reset_steps_to_zero(); 

    LOG_DEBUG("%s Defaults set: StepMode Multiplier- %d, steps per revolution- %d, \n", label_.c_str(), modeMultiplier, steps_per_rev_);

}


// version 0.1.0

void StepperMotor::step_for(int32_t steps, double speed_rpm){
    
    if(steps==0){
        LOG_DEBUG("[%s Step action rejected] steps need to be > 0\n", label_.c_str());
        return;
    }
    
    if(!set_speed(speed_rpm)){
        LOG_DEBUG("[%s Step action rejected] Error in setting speed\n", label_.c_str());

        return;
    }

    if(get_standbyMode()){
        LOG_DEBUG("[%s Step action rejected] Standby mode active\n", label_.c_str());
        return;
    }

    if(active()){
        LOG_DEBUG("[%s Step action rejected] driver task is active\n", label_.c_str());
        return;
    }
    
    driver_.set_direction(steps>=0);
    steps_remaining_ = std::abs(steps);

    add_repeating_timer_us(
        -static_cast<int64_t>(step_period_),
        step_irq_timer_cb,
        this,
        &timer_
    );
}

uint32_t StepperMotor::get_step_period() const{
    return step_period_;
}

bool StepperMotor::set_speed(double rpm){

    if (rpm<=0.0){
        LOG_DEBUG("[%s Error] Speed needs to be > 0rpm:%0.2f", label_.c_str(), rpm);
        return false;
    }
    
    
    uint8_t pulse_width = driver_.get_pulse_width();
    double step_period_us = 60'000'000.0/(rpm * static_cast<double>(steps_per_rev_));
    uint32_t step_period = static_cast<uint32_t>(std::round(step_period_us));
    
    if(step_period <= pulse_width){
        LOG_DEBUG("[%s ERROR] Step period(%u) is <= than Pulse width(%u), Speed not set]", label_.c_str(), step_period, pulse_width);
        return false;
    }

    // uint32_t pulse_interval = step_period - pulse_width; //depreciated
    // driver_.set_pulse_interval(pulse_interval); //depreciated
    
    step_period_ = step_period;
    speed_rpm_ = rpm;

    LOG_DEBUG("%s Speed set to: %0.2f rpm\n", label_.c_str(), rpm);   

    return true;
}

double StepperMotor::get_speed() const{
    return speed_rpm_;
}

bool StepperMotor::get_direction(){
    return driver_.get_direction();
}

void StepperMotor::set_standbyMode(bool active){
    driver_.set_standbyMode(active);
    // update_position(); //depreciated
    reset_steps_to_zero();
    LOG_DEBUG("%s Standby Mode %s", label_.c_str(), active ?"enabled":"disabled");   

}

bool StepperMotor::get_standbyMode() const{
    return driver_.get_standbyMode();
}


void StepperMotor::change_stepMode(StepperDriver::StepMode step_mode){
    driver_.set_stepMode(step_mode);
    auto modeMultiplier = STEP_MODE_MULTIPLIER[static_cast<size_t>(step_mode)];
    steps_per_rev_ = default_steps_per_rev_ * modeMultiplier;
    reset_steps_to_zero();
}

StepperDriver::StepMode StepperMotor::get_stepMode() const{
    return driver_.get_stepMode();
}

bool StepperMotor::active() const {

    return steps_remaining_>0;
}

uint32_t StepperMotor::get_steps_remaining() const{
    return steps_remaining_;
}

void StepperMotor::revolve(double revolutions, double speed_rpm){

    uint32_t steps = static_cast<int32_t>(std::round(revolutions * steps_per_rev_));
    step_for(steps, speed_rpm);

    LOG_DEBUG("%s set for: %.2f revolutions\n", label_.c_str(), revolutions);

}

int32_t StepperMotor::get_position_step() const{
    return position_step_;
}

double StepperMotor::get_position_rev() const{
    return static_cast<double>(position_step_) / steps_per_rev_;
}

double StepperMotor::get_position_deg() const{
    return static_cast<double>(position_step_) / steps_per_rev_ * 360.0;
}

double StepperMotor::get_position_deg_wrapped() const{
    double angle = get_position_deg();
    angle = std::fmod(angle, 360.0);

    if (angle<0.0) {
        angle += 360.0;
    }

    return angle;
}

double StepperMotor::get_position_rad() const{
    return static_cast<double>(position_step_) / steps_per_rev_ * (2.0*M_PI);
}

double StepperMotor::get_position_rad_wrapped() const{
    double angle = get_position_rad();
    angle = std::fmod(angle, 2.0*M_PI);

    if (angle<0.0) {
        angle += 2.0*M_PI;
    }

    return angle;
}

const std::string& StepperMotor::label() const{
    return label_;
};

// private

bool StepperMotor::step_irq_timer_cb(repeating_timer_t* timer){
    StepperMotor* motor = static_cast<StepperMotor*>(timer->user_data);
    motor->driver_.step_high();

    add_alarm_in_us(
        motor->driver_.get_pulse_width(),
        pulse_low_cb,
        motor,
        true
    );

    motor->steps_remaining_--;

    motor->register_step();
    
    return motor->steps_remaining_>0;
}

int64_t StepperMotor::pulse_low_cb(alarm_id_t id, void* user_data){
    auto* motor = static_cast<StepperMotor*>(user_data);
    motor->driver_.step_low();

    return 0;
}

void StepperMotor::register_step(){
    if (driver_.get_direction()){
        position_step_++;
    }else{
        position_step_--;
    }
}

void StepperMotor::reset_steps_to_zero(){
    position_step_ = 0;
}


//Depreciated
// TODO: Phase out and delete non useful members

std::tuple<int32_t, double> StepperMotor::update_position(){
    position_step_ = driver_.get_step_tracker();
    position_revolutions_ = static_cast<double>(position_step_)/static_cast<double>(steps_per_rev_);

    LOG_DEBUG("%s Position: %d steps (%.2f revolutions) \n", label_.c_str(), position_step_, position_revolutions_);

    return std::make_tuple(position_step_,position_revolutions_);  
}

void StepperMotor::update_position(double rev_pos){

    position_step_ = static_cast<int32_t>(std::round(rev_pos * static_cast<double>(steps_per_rev_)));
    position_revolutions_ = static_cast<double>(position_step_)/static_cast<double>(steps_per_rev_);

    driver_.set_step_tracker(position_step_);
    LOG_DEBUG("%s Position: %d steps (%.2f revolutions) \n", label_.c_str(), position_step_, position_revolutions_);
}

void StepperMotor::home(){

    driver_.home();
    update_position();

    LOG_DEBUG("%s HOME HIT! Position reset to: %d steps (%.2f revolutions) \n", label_.c_str(), position_step_, position_revolutions_);   

}

bool StepperMotor::step(){
    bool pulse_flag = driver_.step_pulse();
    
    if (pulse_flag){
        update_position();
    }

    return pulse_flag;
}