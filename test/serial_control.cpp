#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <thread>
#include <chrono>
#include <Eigen/Dense>

int setup_serial(const char* port) {
    int serial = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (serial < 0) return -1;

    fcntl(serial, F_SETFL, 0);

    struct termios tty;
    if (tcgetattr(serial, &tty) != 0) return -1;

    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    tty.c_cflag = CS8 | CREAD | CLOCAL;
    tty.c_lflag = 0;
    tty.c_iflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 5; // 0.5s timeout

    return tcsetattr(serial, TCSANOW, &tty) == 0 ? serial : -1;
}

// Cubic time interpolation function
double cubicInterpolation(double t, double T) {
    return 3 * std::pow(t/T, 2) - 2 * std::pow(t/T, 3);
}

void readSerial(int serial) {
    char buf[512];
    int n = read(serial, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        std::cout << buf << std::flush;
    }
}



int main() {

    const char* port = "/dev/tty.usbmodem1101";
    int serial = setup_serial(port);

    if (serial < 0) {
        std::cerr << "Failed to open serial port.\n";
        return 1;
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Initialize variables
    Eigen::Vector3d curr_pose(0, 0, 0);
    Eigen::Vector3d prev_pose(0, 0, 0);
    Eigen::Vector3d x_i(0, 0, 0); // Initial position
    Eigen::Vector3d x_f(0, 0, 0); // Final position

    constexpr double FREQUENCY = 100.0;               // Hz
    constexpr double PULLEY_TEETH = 20.0;             // Nema17 pulley teeth
    constexpr double BELT_PITCH = 3.0;                // mm/tooth
    constexpr double SCREW_PITCH = 2.0;               // mm/rev

    // Inverse Jacobian to map Cartesian velocity to motor space
    const Eigen::Matrix3d INVERSE_JACOBIAN = (Eigen::Matrix3d() << 
        1.0 / (PULLEY_TEETH * BELT_PITCH), 0, 0,
        0, 1.0 / (PULLEY_TEETH * BELT_PITCH), 0,
        0, 0, 1.0 / SCREW_PITCH
    ).finished();


    while (true) {

        // Read incoming data
        // readSerial(serial);

        // Get user input
        std::cout << "> ";
        std::string user_cmd;
        std::getline(std::cin, user_cmd);
        if (user_cmd == "exit") break;

        // fetch the command from the user
        // example 1:X10.0 Y20.0 Z30.0 T5.0 (with time)
        // example 2:X10.0 Y20.0 Z30.0 F5.0 (with speed)
        // Parse command like: X10.0 Y20.0 Z30.0 T5.0 or F5.0

        float x = 0, y = 0, z = 0, t = -1, f = -1;

        std::istringstream stream(user_cmd);
        std::string token;

        while (stream >> token) {
            char prefix = token[0];
            float value = std::stof(token.substr(1));
            
            switch (prefix) {
                case 'X': x = value; std::cout << "[CMD] X=" << x << " "; break;
                case 'Y': y = value; std::cout << "Y=" << y << " "; break;
                case 'Z': z = value; std::cout << "Z=" << z << " "; break;
                case 'T': t = value; std::cout << "T=" << t << " "; break;
                case 'F': f = value; std::cout << "F=" << f << " "; break;
                default:
                    std::cout << "\n[WARN] unknown field: " << token << "\n";
            }
        }

        std::cout << std::endl;

        if (t > 0 || f > 0) {

            x_f = Eigen::Vector3d(x, y, z); // Set the final position

            if (f > 0) {
                // If speed is specified, calculate time
                t = (x_f - curr_pose).norm() / f;
            }
            
            // 1) build the motion 
            x_i = curr_pose; // Store the current pose
            prev_pose = curr_pose; // Store the previous pose

            Eigen::Vector3d delta_x; // relative motion vector
            Eigen::Vector3d speed_motor; // speed for the motors in RPM
            Eigen::Vector3d rev_motor; // revolutions for the motors

            std::vector<std::string> pico_cmd;
            std::string speed_cmd;
            std::string move_cmd;

            double steps = FREQUENCY * t;

            for (double i = 1; i <= steps; ++i) {

                curr_pose = x_i + cubicInterpolation(i, steps) * (x_f - x_i);
                delta_x = curr_pose - prev_pose;

                speed_motor = (INVERSE_JACOBIAN * delta_x) * FREQUENCY * 60.0; // in RPM
                rev_motor = (INVERSE_JACOBIAN * delta_x);

                // Send the relative motion with pico_command
                speed_cmd = "SPEED x," + std::to_string(speed_motor(0)) + " y," + std::to_string(speed_motor(1)) + " z," + std::to_string(speed_motor(2)) + "\n";
                // std::this_thread::sleep_for(std::chrono::milliseconds(5));

                move_cmd = "MOVE x," + std::to_string(rev_motor(0)) + " y," + std::to_string(rev_motor(1)) + " z," + std::to_string(rev_motor(2)) + "\n";
                // std::this_thread::sleep_for(std::chrono::milliseconds(5));
                pico_cmd.push_back(move_cmd + speed_cmd);

                prev_pose = curr_pose; // Update the previous pose
            }

            // 2) send the motion
            for (const auto& cmd : pico_cmd) {
                std::cout << "[SEND] " << cmd;
                write(serial, cmd.c_str(), cmd.size());
                std::this_thread::sleep_for(std::chrono::milliseconds(9)); // /!\ depends on the frequency
                readSerial(serial); // Read response from the serial port
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    close(serial);
    return 0;
}
