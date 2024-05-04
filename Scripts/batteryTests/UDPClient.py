import socket
from ctypes import *
from enum import Enum
from termcolor import colored
from prettytable import PrettyTable
from PacketTransceiver import *

def send_sample_req(data_stream, server_address):
    payload_out = send_packet(data_stream, server_address, Requests.CLIENT_SAMPLE.value, Responses.NO_RESP.value)
    payload_in = receive_packet(data_stream)
    return payload_out, payload_in

def send_tare_req(data_stream, server_address): 
    send_packet(data_stream, server_address, Requests.CLIENT_TARE.value, Responses.NO_RESP.value)
    receive_packet(data_stream)



def send_calibration_req(data_stream, server_address): 
    response = Responses.BUSY_RESP.value
    prev_time = 0

    #send calibration requests until success or failure response received back from esp32
    while (response != Responses.SERVER_SUCCESS.value) and (response != Responses.SERVER_FAILURE.value):
        try:
            print(colored('Awaiting sucess/fail response.', 'yellow'))

            payload_out = send_packet(data_stream, server_address, Requests.CLIENT_CALIBRATE.value, Responses.NO_RESP.value)
            payload_in = receive_packet(data_stream); 

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
            payload_out = send_packet(data_stream, server_address, Requests.CLIENT_SAMPLE.value, Responses.CLIENT_AFFIRMATIVE.value)
            payload_in = receive_packet(data_stream)

            response = payload_in.response
            print_packet(payload_in, payload_out, payload_in.time_stamp, prev_time)
            prev_time = payload_in.time_stamp

        except socket.timeout:
                print(colored('ERROR: Timeout occurred, resending request...', 'red'))
                continue