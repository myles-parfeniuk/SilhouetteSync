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


# Host and port of the ESP32 UDPServer
server_address_1 = ('192.168.1.81', 49160)

# Create a UDP socket
data_stream = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Set timeout for receiving data
data_stream.settimeout(0.015)  # 15 milliseconds


# use Colorama to make Termcolor work on Windows too
init()

def send_packet(request, response,  server_address):
    global data_stream
    payload_out = Payload(0, 0)
    payload_out.request = request
    payload_out.response = response
    data_stream.sendto(payload_out, server_address)
    return payload_out

def receive_packet():
    global data_stream
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

def send_tare(server_address): 
    send_packet(Requests.TARE_REQ.value, Responses.NULL_RESP.value, server_address)
    receive_packet()



def send_calibration(server_address): 
    response = Responses.BUSY_RESP.value
    prev_time = 0

    #send calibration requests until success or failure response received back from esp32
    while (response != Responses.SUCESS_RESP.value) and (response != Responses.FAILURE_RESP.value):
        try:
            print(colored('Awaiting sucess/fail response.', 'yellow'))

            payload_out = send_packet(Requests.CALIBRATE_REQ.value, Responses.NULL_RESP.value, server_address)
            payload_in = receive_packet(); 

            response = payload_in.response

            # print the received data
            print_packet(payload_in, payload_out, payload_in.time_stamp, prev_time)
            prev_time = payload_in.time_stamp

            if (response == Responses.SUCESS_RESP.value):
                print(colored('Sucessful calibration.', 'green'))
            elif (response == Responses.FAILURE_RESP.value):
                print(colored('Failed calibration.', 'red'))

        except socket.timeout:
                print(colored('ERROR: Timeout occurred, resending request...', 'red'))
                continue
    
    #send sampling request with affirmative response until sampling response receieved 
    while response != Responses.SAMPLING_RESP.value:
        try:
            print(colored('Awaiting sampling response.', 'yellow'))
            payload_out = send_packet(Requests.SAMPLE_REQ.value, Responses.AFFIRMATIVE_RESP.value, server_address)
            payload_in = receive_packet()

            response = payload_in.response
            print_packet(payload_in, payload_out, payload_in.time_stamp, prev_time)
            prev_time = payload_in.time_stamp

        except socket.timeout:
                print(colored('ERROR: Timeout occurred, resending request...', 'red'))
                continue


prev_time = 0

while True:
    try:
        payload_out = send_packet(Requests.SAMPLE_REQ.value, Responses.NULL_RESP.value, server_address_1)
        payload_in = receive_packet()

        # print the received data
        print_packet(payload_in, payload_out, payload_in.time_stamp, prev_time)
        #print(str(prev_time) + " " + str(payload_in.time_stamp) + '\n')
        prev_time = payload_in.time_stamp

  

    
    except socket.timeout:
        print(colored('ERROR: Timeout occurred, resending request...', 'red'))
        continue
    
    #quit 
    if keyboard.is_pressed('q'):
        print(colored('Quitting, socket shutting down.', 'magenta'))
        data_stream.close()
        break  # Exit the loop if 'q' is pressed

    #tare imu  
    if keyboard.is_pressed('t'):
        print(colored('Taring', 'magenta'))
        time.sleep(0.1)
        send_tare(server_address_1)

    #calibrate imu
    if keyboard.is_pressed('c'):
        print(colored('Calibrating', 'magenta'))
        send_calibration(server_address_1)
        time.sleep(0.1)