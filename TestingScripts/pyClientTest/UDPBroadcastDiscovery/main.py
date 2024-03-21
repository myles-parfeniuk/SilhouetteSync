# Third-party and standard packages
import socket
import time
import threading
import keyboard
from ctypes import *
from enum import Enum
import colorama
from colorama import init
from termcolor import colored

# In-house packages
from UDPClient import *
from PacketTransceiver import print_packet, discover_devices

# Use Colorama to make Termcolor work on Windows too
init()


server_addresses = discover_devices()

server_address = server_addresses[0]
# Create a UDP socket
data_stream = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Set timeout for receiving data
data_stream.settimeout(0.015)  # 15 milliseconds

quit_flag = threading.Event() 


def keyboard_input_thread():
    global quit_flag
    global send_tare_flag
    global send_calibration_flag

    while not quit_flag.is_set():
        time.sleep(0.070)

        # check for keyboard input
        if keyboard.is_pressed('q'):
            quit_flag.set()



keyboard_input_thread_hdl = threading.Thread(target = keyboard_input_thread)
keyboard_input_thread_hdl.start()

prev_time = 0
total_time = 0

while not quit_flag.is_set():
    try:

        payload_out, payload_in = send_sample_req(data_stream, server_address)
        # Append received quaternion data to lists
        total_time = total_time + (payload_in.time_stamp/1000 - prev_time/1000)

        # Print the received data
        print_packet(payload_in, payload_out, payload_in.time_stamp, prev_time)
        prev_time = payload_in.time_stamp


    except socket.timeout:
        print(colored('ERROR: Timeout occurred, resending request...', 'red'))
        continue



keyboard_input_thread_hdl.join()
data_stream.close()