import socket
import time
import keyboard  # Import the keyboard module
from ctypes import *

# payload struct, how data is formatted when sent between devices
class Payload(Structure):
    _fields_ = [("id", c_uint8),
                ("x_heading", c_float),
                ("y_heading", c_float),
                ("z_heading", c_float),
                ("accuracy", c_uint8)
                ]

# Create a UDP socket
data_stream = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Set timeout for receiving data
data_stream.settimeout(0.015)  # 15 milliseconds

# Host and port of the ESP32 UDPServers
server_address_1 = ('192.168.168.50', 49160)
server_address_2 = ('192.168.168.248', 49160)
server_address_3 = ('192.168.168.65', 49160)

prev_time_1 = 0
prev_time_2 = 0
prev_time_3 = 0
# grab data endlessly from esp-32
try:
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
            print("Received id={:d}, X_heading={:10.4f}, Y_heading={:10.4f}, Z_Heading={:10.4f}, Accuracy={:d}, TIMESTAMP: {:10.4f}".format(
                payload_in.id,
                payload_in.x_heading,
                payload_in.y_heading,
                payload_in.z_heading,
                payload_in.accuracy,
                (current_time - prev_time_1)/1000000
            ))
            prev_time_1 = current_time

            # send an empty payload (the ESP32 is listening for this, think of it as a request for a measurement)
            payload_out = Payload(0, 0)
            # send to ESP32
            data_stream.sendto(payload_out, server_address_2)

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
            print("Received id={:d}, X_heading={:10.4f}, Y_heading={:10.4f}, Z_Heading={:10.4f}, Accuracy={:d}, TIMESTAMP: {:10.4f}".format(
                payload_in.id,
                payload_in.x_heading,
                payload_in.y_heading,
                payload_in.z_heading,
                payload_in.accuracy,
                (current_time - prev_time_2)/1000000
            ))
            prev_time_2 = current_time

            # send an empty payload (the ESP32 is listening for this, think of it as a request for a measurement)
            payload_out = Payload(0, 0)
            # send to ESP32
            data_stream.sendto(payload_out, server_address_3)

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
            print("Received id={:d}, X_heading={:10.4f}, Y_heading={:10.4f}, Z_Heading={:10.4f}, Accuracy={:d}, TIMESTAMP: {:10.4f}".format(
                payload_in.id,
                payload_in.x_heading,
                payload_in.y_heading,
                payload_in.z_heading,
                payload_in.accuracy,
                (current_time - prev_time_3)/1000000
            ))
            prev_time_3 = current_time
        
        except socket.timeout:
            print("Timeout occurred, resending request...")
            continue
        
        # Check for keyboard input
        if keyboard.is_pressed('q'):
            print("Closing socket...")
            data_stream.close()
            break  # Exit the loop if 'q' is pressed
except KeyboardInterrupt:
    print("KeyboardInterrupt detected, closing socket...")
    data_stream.close()