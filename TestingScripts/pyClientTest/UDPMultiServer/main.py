import socket
import time
import keyboard  # Import the keyboard module
import threading
from ctypes import *
from enum import Enum
import sys
import colorama
from colorama import init
from termcolor import colored
from prettytable import PrettyTable
from UDPClient import *
from PacketTransceiver import print_packet

# Host and port of the ESP32 UDPServers
server_adresses = [
('192.168.1.91', 49160), 
('192.168.1.87', 49160),
('192.168.1.96', 49160)]

prev_times = [0] * len(server_adresses)
data_streams = []
quit_flag = threading.Event() 

def client_thread():
    global data_streams
    global server_adresses
    global prev_times 
    global quit_flag

    while not quit_flag.is_set():

            i = 0
            for server_address in server_adresses:
                try:
                    #request/receieve sample from device i
                    payload_out, payload_in = send_sample_req(data_streams[i], server_address)

                    # print the received data from device i
                    print_packet(payload_in, payload_out, payload_in.time_stamp, prev_times[i])
                    #print(str(prev_times[i]) + " " + str(payload_in.time_stamp) + '\n')
                    prev_times[i] = payload_in.time_stamp
                    i += 1

                except socket.timeout:
                    print(colored('ERROR: Timeout occurred, resending request...', 'red'))
                    continue

def keyboard_input_thread():
    global quit_flag

    while not quit_flag.is_set():
        time.sleep(0.070)

        # check for keyboard input
        if keyboard.is_pressed('q'):
            quit_flag.set()


#create sockets
for address in server_adresses:
    data_streams.append(socket.socket(socket.AF_INET, socket.SOCK_DGRAM))
    data_streams[-1].settimeout(0.015)  # 15 milliseconds

#initialize colorama
init()


client_thread_hdl = threading.Thread(target = client_thread) 
keyboard_input_thread_hdl = threading.Thread(target = keyboard_input_thread)

client_thread_hdl.start()
keyboard_input_thread_hdl.start()

# grab data until 'q' key is hit
while not quit_flag.is_set():
    time.sleep(1)



print(colored('Quitting, socket shutting down.', 'magenta'))
for data_stream in data_streams:
    data_stream.close()

client_thread_hdl.join()
keyboard_input_thread_hdl.join()

     