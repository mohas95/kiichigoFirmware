#include "MotionPlanner.h"
#include "Log.h"
#include <stdio.h>
#include <string.h>


MotionPlanner::MotionPlanner(const MotionConfig& config) {
    
    for (StepperMotor* stepper : config.stepper_motors){
        stepper_motors_[stepper->label()] = stepper; 
    }

}

void MotionPlanner::process_command(const char* line){
    char command[16];
    int value;

    if (sscanf(line, "%15s %d", command, &value) == 2) {
        printf("Parsed command: [%s] with value: %d\n", command, value);

        if (strcmp(command, "MOVE") == 0) {
            // Do something like: motor.revolve(value);
            printf("➡️  Executing MOVE %d\n", value);
        } else {
            printf("⚠️  Unknown command: %s\n", command);
        }

    } else {
        printf("⚠️  Could not parse input: %s\n", line);
    }

}


void MotionPlanner::request_action(){
    int c = getchar_timeout_us(0);
    
    if (c != PICO_ERROR_TIMEOUT) {
            printf("Received: %c\n", c);
    }
    
}

void MotionPlanner::loop_forever(){

while (true){

    bool active = load_action_queue();

    start_action();

    while(active){
        request_action();
        update_action();


    }

    
    if


}

}