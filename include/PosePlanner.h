#pragma once
#include <Eigen/Dense>
#include <string>
#include <vector>

constexpr double COMMUNICATION_FREQUENCY = 100.0; // Hz
constexpr double PULLEY_TEETH = 20.0;             // Nema17 pulley teeth
constexpr double BELT_PITCH = 3.0;                // mm/tooth
constexpr double SCREW_PITCH = 2.0;               // mm/rev

// Inverse Jacobian to map Cartesian velocity to motor space
const Eigen::Matrix3d INVERSE_JACOBIAN = (Eigen::Matrix3d() << 
    1.0 / (PULLEY_TEETH * BELT_PITCH), 0, 0,
    0, 1.0 / (PULLEY_TEETH * BELT_PITCH), 0,
    0, 0, 1.0 / SCREW_PITCH
).finished();

class PosePlanner {
public:
    PosePlanner();

    // Motion planning interfaces
    void QueueRelativeMotion(Eigen::Vector3d xf, double T);
    void QueueAbsoluteMotion(Eigen::Vector3d xf, double T);
    void LiveQueueRelativeMotion(Eigen::Vector3d xf, double T);
    void LiveQueueAbsoluteMotion(Eigen::Vector3d xf, double T);

    // G-code interfaces
    void executeGCode(std::string GcodeLine);
    void GCodeFromTerminal();
    void GCodeFromFile(const std::string& filepath);

    // Pose and frequency
    Eigen::Vector3d curr_pose;
    double frequency;

private:
    // Internal variables
    Eigen::Vector3d xi;
    Eigen::Vector3d prev_pose;
    Eigen::Vector3d rev_motor;
    Eigen::Vector3d speed_motor;
    Eigen::Matrix3d inv_jacobian;

    // Helpers
    double cubicInterpolation(double t, double T);
};
