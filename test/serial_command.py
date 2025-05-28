import serial
import time

# === CONFIGURATION ===
SERIAL_PORT = '/dev/ttyACM0'  # Replace with your actual port, e.g. 'COM3' on Windows
BAUD_RATE = 115200              # Match your device's Serial.begin()
COMMAND_FILE = 'motor_commands.txt'
DELAY_BETWEEN_COMMANDS = 0  # Seconds between sending each command
TIMEOUT = 2                   # Serial read timeout in seconds

# === OPEN SERIAL CONNECTION ===
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=TIMEOUT)
time.sleep(2)  # Wait for the device to reset

print(f"🔌 Connected to {SERIAL_PORT}")

try:
    with open(COMMAND_FILE, 'r') as file:
        for line_number, line in enumerate(file, 1):
            command = line.strip()
            if not command:
                continue  # Skip empty lines

            # Send command
            ser.write((command + '\n').encode('utf-8'))
            print(f"📤 Sent (line {line_number}): {command}")

            # Read response if available
            time.sleep(DELAY_BETWEEN_COMMANDS)
            while ser.in_waiting:
                response = ser.readline().decode('utf-8', errors='replace').strip()
                print(f"💬 Received: {response}")

except FileNotFoundError:
    print(f"❌ File not found: {COMMAND_FILE}")
except KeyboardInterrupt:
    print("⛔ Interrupted by user.")
finally:
    ser.close()
    print("🔌 Serial connection closed.")
