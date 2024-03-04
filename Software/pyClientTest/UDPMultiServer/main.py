import socket
import time
import keyboard  # Import the keyboard module
from ctypes import *
from enum import Enum
import sys
import colorama
from colorama import init
from termcolor import colored
from prettytable import PrettyTable

class Requests(Enum):
    SAMPLE_REQ = 0
    TARE_REQ = 1
    CALIBRATE_REQ = 2

class Responses(Enum):
    BUSY_RESP = 255
    SUCESS_RESP = 254
    FAILURE_RESP = 253
    AFFIRMATIVE_RESP = 252
    NULL_RESP = 251
    SAMPLING_RESP = 0
# payload struct, how data is formatted when sent between devices
class Payload(Structure):
    _fields_ = [("request", c_uint8),
                ("response", c_uint8),
                ("id", c_uint8),
                ("quat_i", c_float),
                ("quat_j", c_float),
                ("quat_k", c_float),
                ("quat_real", c_float),
                ("accuracy", c_uint8),
                ("time_stamp", c_uint64)
                ]


# Host and port of the ESP32 UDPServers
server_adresses = [
('192.168.1.81', 49160), 
('192.168.1.90', 49160)]

prev_times = [0] * len(server_adresses)
data_streams = []
#create sockets
for address in server_adresses:
    data_streams.append(socket.socket(socket.AF_INET, socket.SOCK_DGRAM))
    data_streams[-1].settimeout(0.015)  # 15 milliseconds


init()

def send_packet(data_stream, request, response,  server_address):
    payload_out = Payload(0, 0)
    payload_out.request = request
    payload_out.response = response
    data_stream.sendto(payload_out, server_address)
    return payload_out

def receive_packet(data_stream):
    payload_in = Payload(0, 0)

    buff, _ = data_stream.recvfrom(sizeof(Payload))

    if len(buff) != sizeof(Payload):
        print(colored('ERROR: Received buffer size does not match expected payload size.', 'red'))

    payload_in = Payload.from_buffer_copy(buff)

    return payload_in

def print_packet(payload_in, payload_out, current_time, prev_time):

    packet_table = PrettyTable(['Packet Direction', 'Timestamp (ms)', 'Request', 'Response', 'ID', 'Quat I', 'Quat J', 'Quat K', 'Quat Real', 'Accuracy'])

    timestamp = "N/A"
    request = Requests(payload_out.request).name
    response = Responses(payload_out.response).name
    id_value = "{:d}".format(payload_out.id)
    quat_i = "{:.4f}".format(payload_out.quat_i)
    quat_j = "{:.4f}".format(payload_out.quat_j)
    quat_k = "{:.4f}".format(payload_out.quat_k)
    quat_real = "{:.4f}".format(payload_out.quat_real)
    accuracy = "{:d}".format(payload_out.accuracy)

    packet_table.add_row(['Out', timestamp, request, response, id_value, quat_i, quat_j, quat_k, quat_real, accuracy])

    timestamp = "{:.4f}".format((current_time - prev_time)/1000)
    request = Requests(payload_in.request).name
    response = Responses(payload_in.response).name
    id_value = "{:d}".format(payload_in.id)
    quat_i = "{:.4f}".format(payload_in.quat_i)
    quat_j = "{:.4f}".format(payload_in.quat_j)
    quat_k = "{:.4f}".format(payload_in.quat_k)
    quat_real = "{:.4f}".format(payload_in.quat_real)
    accuracy = "{:d}".format(payload_in.accuracy)

    packet_table.add_row(['In', timestamp, request, response, id_value, quat_i, quat_j, quat_k, quat_real, accuracy])
    
    print(colored(packet_table, 'white'))


# grab data until 'q' key is hit
while True:
    try:
        i = 0
        for server_address in server_adresses:
            #request/receieve sample from device i
            payload_out = send_packet(data_streams[i], Requests.SAMPLE_REQ.value, Responses.NULL_RESP.value, server_address)
            payload_in = receive_packet(data_streams[i])

            # print the received data from device i
            print_packet(payload_in, payload_out, payload_in.time_stamp, prev_times[i])
            #print(str(prev_times[i]) + " " + str(payload_in.time_stamp) + '\n')
            prev_times[i] = payload_in.time_stamp
            i += 1

        # Check for keyboard input
        if keyboard.is_pressed('q'):
            print(colored('Quitting, socket shutting down.', 'magenta'))
            data_stream.close()
            break  # Exit the loop if 'q' is pressed


        
    except socket.timeout:
        print(colored('ERROR: Timeout occurred, resending request...', 'red'))
        continue
        
     
