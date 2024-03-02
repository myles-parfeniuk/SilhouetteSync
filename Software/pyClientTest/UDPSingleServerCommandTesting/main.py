import socket
import time
import keyboard  # Import the keyboard module
from ctypes import *

# payload struct, how data is formatted when sent between devices
class Payload(Structure):
    _fields_ = [("request", c_uint8),
                ("id", c_uint8),
                ("quat_i", c_float),
                ("quat_j", c_float),
                ("quat_k", c_float),
                ("quat_real", c_float),
                ("accuracy", c_uint8)
                ]

# Create a UDP socket
data_stream = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Set timeout for receiving data
data_stream.settimeout(0.015)  # 15 milliseconds

# Host and port of the ESP32 UDPServer
server_address_1 = ('192.168.1.91', 49160)

prev_time_1 = 0

# grab data endlessly from esp-32


def send_tare(): 
    global data_stream
    global server_address_1
    payload_out = Payload(0, 0)
    payload_out.request = 1
    data_stream.sendto(payload_out, server_address_1)

    # receive IMU data from ESP32
    buff, _ = data_stream.recvfrom(sizeof(Payload))
    print("Received buffer size: {:d} bytes".format(len(buff)))
    if len(buff) != sizeof(Payload):
        print("Received buffer size does not match expected payload size.")

def send_calibration(): 
    global data_stream
    global server_address_1
    payload_out = Payload(0, 0)
    payload_in = Payload(0, 0)
    payload_out.request = 2
    data_stream.sendto(payload_out, server_address_1)

while True:
    try:

        # send an empty payload (the ESP32 is listening for this, think of it as a request for a measurement)
        payload_out = Payload(0, 0)
        # send to ESP32
        data_stream.sendto(payload_out, server_address_1)

        # receive IMU data from ESP32
        buff, _ = data_stream.recvfrom(sizeof(Payload))
        print("Received buffer size: {:d} bytes".format(len(buff)))
        if len(buff) != sizeof(Payload):
            print("Received buffer size does not match expected payload size.")
            continue

        # Parse the received data as Payload
        payload_in = Payload.from_buffer_copy(buff)

        # print the received data
        current_time = time.time_ns()
        print("Request={:d}, Received id={:d}, Quat_I={:10.4f}, Quat_J={:10.4f}, Quat_K={:10.4f}, Quat_Real={:10.4f}, Accuracy={:d}, TIMESTAMP: {:10.4f}".format(
            payload_in.request,
            payload_in.id,
            payload_in.quat_i,
            payload_in.quat_j,
            payload_in.quat_k,
            payload_in.quat_real,
            payload_in.accuracy,
            (current_time - prev_time_1)/1000000
        ))
        prev_time_1 = current_time

    
    except socket.timeout:
        print("Timeout occurred, resending request...")
        continue
    
    # Check for keyboard input
    if keyboard.is_pressed('q'):
        print("Closing socket...")
        data_stream.close()
        break  # Exit the loop if 'q' is pressed
    if keyboard.is_pressed('t'):
        print("Taring")
        time.sleep(0.1)
        send_tare()

    if keyboard.is_pressed('c'):
        send_calibration()
        time.sleep(0.1)