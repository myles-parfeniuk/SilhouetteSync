import socket
import sys
import time
from ctypes import *


# payload struct, how data is formatted when sent between devices
class Payload(Structure):
    _fields_ = [("id", c_uint8),
                ("x_heading", c_float),
                ("y_heading", c_float),
                ("z_heading", c_float),
                ("accuracy", c_uint8)
                ]


# Create a TCP/IP sockets
imu_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# when the esp-32 boots up the TCPServer firmware will print an ip & port number over the serial console, check that, it will remain static until the device is reflashed
host = '192.168.1.90'
port = 49160
server_address = (host, port)
# Connect the socket to the port where the server is listening
imu_socket.connect(server_address)
prev_time = 0
# grab data endlessly from esp-32
while True:
    # send an empty payload (the esp-32 is listening for this, think of it as a request for a measurement)
    payload_out = Payload(0, 0)
    # send to esp-32
    nsent = imu_socket.send(payload_out)
    print("Sent {:d} bytes".format(nsent))
    # receive imu data from esp-32
    buff = imu_socket.recv(sizeof(Payload))
    payload_in = Payload.from_buffer_copy(buff)
    # print the received data
    current_time = time.time_ns()
    print("Received id={:d}, X_heading={:10.4f}, Y_heading={:10.4f}, Z_Heading={:10.4f}, Accuracy={:d}, TIMESTAMP: {:10.4f}".format(payload_in.id,
                                                                                       payload_in.x_heading,
                                                                                       payload_in.y_heading,
                                                                                       payload_in.z_heading,
                                                                                       payload_in.accuracy,
                                                                                       (current_time - prev_time)/1000000))
    prev_time = current_time




















