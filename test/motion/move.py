import serial
import time
import sys
import numpy as np
from functions import motion_planning, send_commands


############## SERIAL COMMUNICATION #############
SERIAL_PORT = '/dev/ttyACM0'
BAUD_RATE = 115200
DELAY_BETWEEN_COMMANDS = 0.002
TIMEOUT = 2

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=TIMEOUT)
time.sleep(2)
print(f"Connected to {SERIAL_PORT}")

############## HOMING #############
print("Standby OFF")
command = "STANDBY x,false y,false z,false"
ser.write((command + '\n').encode('utf-8'))

print("Homing...")
command = "MOVE x,-100 y,100 z,100"
ser.write((command + '\n').encode('utf-8'))

curr_pos = np.array([0.0, 0.0, 0.0])

############## MOTION FROM G-CODE #############
filename = sys.argv[1]
file_path = f"/home/pi/motion/path/{filename}"

try:
    with open(file_path, 'r') as file:

        for line in file:

            line = line.strip()

            
            target_pos = curr_pos.copy()
            T = 0.0
            motor_cmds = []

            for part in line.split():
                if part.startswith("X"):
                    target_pos[0] = float(part[1:])
                elif part.startswith("Y"):
                    target_pos[1] = float(part[1:])
                elif part.startswith("Z"):
                    target_pos[2] = float(part[1:])
                elif part.startswith("T"):
                    T = float(part[1:])

            ########## VALIDATION ##########
            if T <= 0:
                print("Invalid or missing T:", line)
                continue

            if np.allclose(curr_pos, target_pos):
                continue
        
            ###### INVERSE KINEMATICS CALCULATION ######
            motor_cmds = motion_planning(curr_pos, target_pos, T)
            done = send_commands(ser, motor_cmds, delay=DELAY_BETWEEN_COMMANDS)

            if done:
                print(f"Moved to position: X{target_pos[0]} Y{target_pos[1]} Z{target_pos[2]}")

            curr_pos = target_pos

except FileNotFoundError:
    print(f"File not found: {file_path}")

except KeyboardInterrupt:
    print("Interrupted by user.")

finally:
    print("Standby ON")
    command = "STANDBY x,true y,true z,true"
    ser.write((command + '\n').encode('utf-8'))

    ser.close()
    print("Serial connection closed.")