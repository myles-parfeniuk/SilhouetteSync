# Third-party and standard packages
import socket
import time
import threading
import keyboard
import numpy as np  # Import NumPy for array manipulation
from ctypes import *
from enum import Enum
import colorama
from colorama import init
from termcolor import colored
import matplotlib.pyplot as plt  # Importing Matplotlib for plotting

# In-house packages
from UDPClient import *
from PacketTransceiver import print_packet

# Host and port of the ESP32 UDPServer
server_address = ('192.168.1.91', 49160)

# Create a UDP socket
data_stream = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Set timeout for receiving data
data_stream.settimeout(0.015)  # 15 milliseconds

quit_flag = threading.Event() 
send_tare_flag = threading.Event()
send_calibration_flag = threading.Event()


# Use Colorama to make Termcolor work on Windows too
init()

# Initialize lists to store quaternion data
time_stamps = []
quat_i_data = []
quat_j_data = []
quat_k_data = []
quat_data = []
prev_time = 0

# Enable interactive mode for real-time plotting
plt.ion()
fig, ax = plt.subplots(figsize=(8, 6))  # Create a single subplot
lines_i, = ax.plot([], [], label='quat_i')  # Line for quat_i
lines_j, = ax.plot([], [], label='quat_j')  # Line for quat_j
lines_k, = ax.plot([], [], label='quat_k')  # Line for quat_k
lines_real, = ax.plot([], [], label='quat_real')  # Line for quat_real
ax.legend()
ax.set_xlabel('Time Stamp')
ax.set_ylabel('Quaternion Value')
ax.set_title('Quaternion Data Over Time')

plt.tight_layout()
plt.show()
ax.set_xlim(0, 10000)
ax.set_ylim(-1, 1)
total_time = 0


def keyboard_input_thread():
    global quit_flag
    global send_tare_flag
    global send_calibration_flag

    while not quit_flag.is_set():
        time.sleep(0.070)

        # check for keyboard input
        if keyboard.is_pressed('q'):
            quit_flag.set()

        if keyboard.is_pressed('t'):
            send_tare_flag.set()

        if keyboard.is_pressed('c'):
            send_calibration_flag.set()


keyboard_input_thread_hdl = threading.Thread(target = keyboard_input_thread)
keyboard_input_thread_hdl.start()

while not quit_flag.is_set():
    try:
        if total_time > 10000:
            total_time = 0
            time_stamps.clear()
            quat_data.clear()

        payload_out, payload_in = send_sample_req(data_stream, server_address)
        # Append received quaternion data to lists
        total_time = total_time + (payload_in.time_stamp/1000 - prev_time/1000)
        time_stamps.append(total_time)
        quat_data.append((payload_in.quat_i, payload_in.quat_j, payload_in.quat_k, payload_in.quat_real))

        # Print the received data
        print_packet(payload_in, payload_out, payload_in.time_stamp, prev_time)
        prev_time = payload_in.time_stamp

        # Update the plot with new data for each component
        quat_data_array = np.array(quat_data)
        lines_i.set_data(time_stamps, quat_data_array[:, 0])  # Update quat_i data
        lines_j.set_data(time_stamps, quat_data_array[:, 1])  # Update quat_j data
        lines_k.set_data(time_stamps, quat_data_array[:, 2])  # Update quat_k data
        lines_real.set_data(time_stamps, quat_data_array[:, 3])  # Update quat_real data

        # Draw the plot
        plt.draw()
        plt.pause(0.001)  # Pause to allow time for the plot to update

    except socket.timeout:
        print(colored('ERROR: Timeout occurred, resending request...', 'red'))
        continue

    # Tare IMU
    if send_tare_flag.is_set():
        print(colored('Taring', 'magenta'))
        time.sleep(0.1)
        send_tare_req(data_stream, server_address)
        send_tare_flag.clear()

    # Calibrate IMU
    if send_calibration_flag.is_set():
        print(colored('Calibrating', 'magenta'))
        send_calibration_req(data_stream, server_address)
        time.sleep(0.1)
        send_calibration_flag.clear()

keyboard_input_thread_hdl.join()
data_stream.close()