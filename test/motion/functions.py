import time
import numpy as np

# X
BELT_PITCH_X = 2     # mm/teeth
PULLEY_TEETH_X = 32  # teeth/rev
MM_PER_REV_X = BELT_PITCH_X * PULLEY_TEETH_X

# Y
BELT_PITCH_Y = 2     # mm/teeth
PULLEY_TEETH_Y = 32  # teeth/rev
MM_PER_REV_Y = BELT_PITCH_Y * PULLEY_TEETH_Y

# Z 
MM_PER_REV_Z = 2 # mm/rev

# Jacobian
CONVERSION = np.array([[MM_PER_REV_X, 0, 0],  # Motor 1
                       [0, MM_PER_REV_Y, 0],  # Motor 2
                       [0, 0, MM_PER_REV_Z]]) # Motor 3

# Inverse Jacobian
CONVERSION_INVERSE = np.array([[1/MM_PER_REV_X, 0, 0],  # Motor 1
                               [0, 1/MM_PER_REV_Y, 0],  # Motor 2
                               [0, 0, 1/MM_PER_REV_Z]]) # Motor 3

UPDATE_FREQUENCY = 100  # Hz

def s(t, T):
    tau = t / T
    return 3 * (tau**2) - 2 * (tau**3)


def motion_planning(x_i, x_f, T):
    
    cmd_motor = []
    prev_x = x_i

    N = int(UPDATE_FREQUENCY * T)

    for t in range(1, N + 1):

        x = x_i + (x_f - x_i) * s(t, N)
        delta_x = x - prev_x

        rev_motor = CONVERSION_INVERSE @ delta_x
        speed_motor = CONVERSION_INVERSE @ (UPDATE_FREQUENCY *  delta_x)

        cmd_motor.append({"rev": rev_motor, "speed": speed_motor})
        prev_x = x

    return cmd_motor


def send_commands(ser, commands, delay=0.01):

    for command in commands:
        speed_cmd = f"SPEED x,{command['speed'][0]:.6f} y,{command['speed'][1]:.6f} z,{command['speed'][2]:.6f} "
        ser.write((speed_cmd + '\n').encode('utf-8'))
        time.sleep(delay)
        move_cmd = f"MOVE x,{command['rev'][0]:.6f} y,{command['rev'][1]:.6f} z,{command['rev'][2]:.6f} "
        ser.write((move_cmd + '\n').encode('utf-8'))
        time.sleep(delay)

    return True