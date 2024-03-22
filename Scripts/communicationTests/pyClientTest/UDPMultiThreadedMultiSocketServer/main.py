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
client_threads = []

def client_thread(data_stream, server_address, prev_time):
    global quit_flag

    while not quit_flag.is_set():
        try:
            #request/receieve sample from device i
            payload_out, payload_in = send_sample_req(data_stream, server_address)

            # print the received data from device i
            print_packet(payload_in, payload_out, payload_in.time_stamp, prev_time)
            #print(str(prev_times[i]) + " " + str(payload_in.time_stamp) + '\n')
            prev_time = payload_in.time_stamp

        except socket.timeout:
            print(colored('ERROR: Timeout occurred, resending request...', 'red'))
            continue

    data_stream.close()

def keyboard_input_thread():
    global quit_flag

    while not quit_flag.is_set():
        time.sleep(0.070)

        # check for keyboard input
        if keyboard.is_pressed('q'):
            quit_flag.set()



#initialize colorama
init()

#create sockets
for address in server_adresses:
    data_streams.append(socket.socket(socket.AF_INET, socket.SOCK_DGRAM))
    data_streams[-1].settimeout(0.015)  # 15 milliseconds

#create threads
for i, address in enumerate(server_adresses):
    client_threads.append(threading.Thread(target=client_thread, args=(data_streams[i], address, prev_times[i])))

keyboard_input_thread_hdl = threading.Thread(target = keyboard_input_thread)


#initialize colorama
init()

for thread in client_threads:
    thread.start()

keyboard_input_thread_hdl.start()

# grab data until 'q' key is hit
while not quit_flag.is_set():
    time.sleep(1)



print(colored('Quitting, socket shutting down.', 'magenta'))
for thread in client_threads:
    thread.join()

keyboard_input_thread_hdl.join()

     
