// PosePlanner.cpp
#include "PosePlanner.h"
#include "MotionPlanner.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <chrono>
#include <Eigen/Dense>

PosePlanner::PosePlanner() {

    prev_pose = Eigen::Vector3d::Zero();
    curr_pose = Eigen::Vector3d::Zero();
    rev_motor = Eigen::Vector3d::Zero();
    speed_motor = Eigen::Vector3d::Zero();

    frequency = COMMUNICATION_FREQUENCY;
    inv_jacobian = INVERSE_JACOBIAN;
}

// Cubic time interpolation function
double cubicInterpolation(double t, double T) {
    return 3 * std::pow(t/T, 2) - 2 * std::pow(t/T, 3);
}

// Version that queues all actions at once using the MOVE command
void PosePlanner::QueueRelativeMotion(Eigen::Vector3d xf, double T) {

    xi = curr_pose; // Store the current pose
    prev_pose = curr_pose; // Store the previous pose

    int steps = static_cast<int>(frequency * T);

    for (int t = 1; t <= steps; ++t) {

        double s = cubicInterpolation(static_cast<double>(t), static_cast<double>(steps));

        curr_pose = xi + s * (xf - xi);
        Eigen::Vector3d delta_x = curr_pose - prev_pose;

        Eigen::Vector3d speed_motor = inv_jacobian * (frequency * delta_x); // in rev/s
        Eigen::Vector3d relative_rev_motor = inv_jacobian * delta_x * 60.0;  // in RPM
        
        // Send the relative motion via MOVE command
       
        prev_pose = curr_pose; // Update the previous pose
    }
}

// Version that queues all actions at once using the POSITION command (more precise version)
void PosePlanner::QueueAbsoluteMotion(Eigen::Vector3d xf, double T) {
    
    int steps = static_cast<int>(frequency * T);

    xi = curr_pose; // Store the previous pose
    prev_pose = curr_pose; // Store the previous pose

    for (int t = 1; t <= steps; ++t) {

        double s = cubicInterpolation(static_cast<double>(t), static_cast<double>(steps));

        curr_pose = xi + s * (xf - xi);
        Eigen::Vector3d delta_x = curr_pose - prev_pose;

        Eigen::Vector3d speed_motor = inv_jacobian * (frequency * delta_x); // in rev/s
        Eigen::Vector3d absolute_rev_motor = inv_jacobian * curr_pose * 60.0;  // in RPM
        
        // Send the absolute POSITION command (not implemented yet) 
        
        prev_pose = curr_pose; // Update the previous pose
    }
}

// queues the action in real-time with a relative motion
void PosePlanner::LiveQueueRelativeMotion(Eigen::Vector3d xf, double T) {

    xi = curr_pose; // Store the previous pose
    prev_pose = curr_pose; // Store the previous pose

    using clock = std::chrono::steady_clock;

    auto start_time = clock::now();
    auto next_loop_time = start_time;

    while (std::chrono::duration<double>(next_loop_time - start_time).count() < T) {
        
        auto now = clock::now();
        double elapsed_time = std::chrono::duration<double>(now - start_time).count();

        double s = cubicInterpolation(elapsed_time, T);
        curr_pose = xi + s * (xf - xi);

        Eigen::Vector3d delta_x = curr_pose - prev_pose;
        Eigen::Vector3d speed_motor = inv_jacobian * (frequency * delta_x);
        Eigen::Vector3d rev_motor = inv_jacobian * delta_x * 60.0;

        // Send SPEED and MOVE command here...

        prev_pose = curr_pose;

        next_loop_time += std::chrono::duration<double>(1.0 / frequency);
        std::this_thread::sleep_until(next_loop_time);
    }
}

// queues the action in real-time with an Absolute motion command
void PosePlanner::LiveQueueAbsoluteMotion(Eigen::Vector3d xf, double T) {

    xi = curr_pose; // Store the previous pose
    prev_pose = curr_pose; // Store the previous pose

    using clock = std::chrono::steady_clock;

    auto start_time = clock::now();
    auto next_loop_time = start_time;

    while (std::chrono::duration<double>(next_loop_time - start_time).count() < T) {
        
        auto now = clock::now();
        double elapsed_time = std::chrono::duration<double>(now - start_time).count();

        double s = cubicInterpolation(elapsed_time, T);
        curr_pose = xi + s * (xf - xi);

        Eigen::Vector3d delta_x = curr_pose - prev_pose;
        Eigen::Vector3d speed_motor = inv_jacobian * (frequency * delta_x);
        Eigen::Vector3d rev_motor = inv_jacobian * curr_pose * 60.0;

        // Send SPEED and POSITION command here...

        prev_pose = curr_pose;

        next_loop_time += std::chrono::duration<double>(1.0 / frequency);
        std::this_thread::sleep_until(next_loop_time);
    }
}

// the G-code take into only the speed of the displacement
// while the speed can be recover with the time and the displacement, 
// it's is sometimes useful to have the time directly 
// so TX refers a to the time X that the motion will take to be competed

void PosePlanner::executeGCode(std::string GcodeLine) {
    std::vector<std::string> tokens;
    std::istringstream stream(GcodeLine);
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }

    std::string Gkey = "G01";
    double x = curr_pose.x();
    double y = curr_pose.y();
    double z = curr_pose.z();
    double T = -1.0;
    double F = -1.0;

    for (const auto& token : tokens) {
        if (token[0] == 'G') Gkey = token;
        else if (token[0] == 'X') x = std::stod(token.substr(1));
        else if (token[0] == 'Y') y = std::stod(token.substr(1));
        else if (token[0] == 'Z') z = std::stod(token.substr(1));
        else if (token[0] == 'T') T = std::stod(token.substr(1));
        else if (token[0] == 'F') F = std::stod(token.substr(1));
        else std::cerr << "Unknown token: " << token << std::endl;
    }

    Eigen::Vector3d target(x, y, z);

    if (T < 0.0 && F > 0.0) {
        double distance = (target - curr_pose).norm();
        T = distance / F;
    }

    if (T > 0.0) {
        if (Gkey == "G01" || Gkey == "G1") {
            QueueRelativeMotion(target, T);
        } else {
            std::cerr << "Unsupported G-code command: " << Gkey << std::endl;
        }
    } else {
        std::cerr << "Invalid G-code input. T or F must be specified." << std::endl;
    }
}

void PosePlanner::GCodeFromTerminal() {
    // example 1:G01 X10.0 Y20.0 Z30.0 T5.0 (with time)
    // example 2:G01 X10.0 Y20.0 Z30.0 F5.0 (with speed)
    std::string line;
    std::cout << "Enter G-code commands (type 'exit' to quit):" << std::endl;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, line);
        if (line == "exit") break;
        if (line.empty()) continue;

        executeGCode(line);
    }
}

void PosePlanner::GCodeFromFile(const std::string& filepath) {

    std::ifstream file(filepath);

    if (!file.is_open()) {
        std::cerr << "Error: could not open file '" << filepath << "'" << std::endl;
        return;
    }

    std::string line;

    while (std::getline(file, line)) {
        
        // Remove trailing carriage return (Windows-style line endings)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // Skip comments and empty lines
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;

        std::cout << "Executing: " << line << std::endl;
        executeGCode(line);
    }

    file.close();
}
