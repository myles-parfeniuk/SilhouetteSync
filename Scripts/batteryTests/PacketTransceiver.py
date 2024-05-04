import socket
from ctypes import *
from enum import Enum
from termcolor import colored
from prettytable import PrettyTable

class Requests(Enum):
    CLIENT_DISCOVERY = 0
    CLIENT_SAMPLE = 1
    CLIENT_TARE = 2
    CLIENT_CALIBRATE = 3

class Responses(Enum):
    SERVER_DISCOVERED = 0
    SERVER_SAMPLING = 1
    SERVER_FAILURE = 2
    SERVER_SUCCESS = 3
    SERVER_BUSY = 4
    CLIENT_AFFIRMATIVE = 5
    NO_RESP = 6

class PowerStates(Enum):
    USB_POWERED_CHARGING = 0
    USB_POWERED_FULLY_CHARGED = 1
    BATTERY_POWERED = 2

class Payload(Structure):
    _fields_ = [("request", c_uint8),
                ("response", c_uint8),
                ("id", c_char * 30),
                ("quat_i", c_float),
                ("quat_j", c_float),
                ("quat_k", c_float),
                ("quat_real", c_float),
                ("accuracy", c_uint8),
                ("time_stamp", c_uint64),
                ("retransmit_delay", c_uint8),
                ("battery_voltage", c_float),
                ("battery_soc", c_uint8),
                ("power_state", c_uint8)
                ]


def discover_devices():
    # create a UDP socket for broadcasting
    broadcast_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # allow re-using of socket address
    broadcast_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    # enable broadcasting on socket
    broadcast_socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

    # Set timeout for receiving responses
    broadcast_socket.settimeout(2)  # 2 second timeout

    #create discovery request packet
    broadcast_payload = Payload(0,0)
    broadcast_payload.request = Requests.CLIENT_DISCOVERY.value
    broadcast_payload.response = Responses.NO_RESP.value

    broadcast_address = ('<broadcast>', 49160)  

    server_addresses = []
    retry_num = 10; #discovery attempts (broadcast packet send + receive)
    max_devices = 1; 


    for i in range(retry_num):
        try:
            # broadcast the discovery request message
            broadcast_socket.sendto(broadcast_payload, broadcast_address)
            payload_in, addr = broadcast_socket.recvfrom(sizeof(Payload))
            print(colored('Broadcast response received, from ip ' + addr[0], 'green'))

            unique_addr = True
            for i in range(len(server_addresses)):
                if addr == server_addresses[i]:
                    unique_addr = False

            if unique_addr:      
                server_addresses.append(addr)
                
            if len(server_addresses) >= max_devices:
                break

        except socket.timeout:
            print(colored('Broadcast socket timeout occurred, ' + str(len(server_addresses)) + ' devices found.', 'yellow'))

    return server_addresses

def send_packet(udp_socket, server_address, request, response):
    payload_out = Payload(0, 0)
    payload_out.request = request
    payload_out.response = response
    payload_out.retransmit_delay = 10
    udp_socket.sendto(payload_out, server_address)
    return payload_out

def receive_packet(udp_socket):
    payload_in = Payload(0, 0)

    buff, _ = udp_socket.recvfrom(sizeof(Payload))

    if len(buff) != sizeof(Payload):
        print(colored('ERROR: Received buffer size does not match expected payload size.', 'red'))

    payload_in = Payload.from_buffer_copy(buff)

    return payload_in

def print_packet(payload_in, payload_out, current_time, prev_time):
    packet_table = PrettyTable(['Packet Direction', 'Timestamp (ms)', 'Request', 'Response', 'Hardware ID', 'Quat I', 'Quat J', 'Quat K', 'Quat Real', 'Accuracy', 'PWRState', 'VBat (mv)', 'SOC (%)'])

    timestamp = "N/A"
    request = Requests(payload_out.request).name
    response = Responses(payload_out.response).name
    id_value = payload_out.id.decode()
    quat_i = "{:.4f}".format(payload_out.quat_i)
    quat_j = "{:.4f}".format(payload_out.quat_j)
    quat_k = "{:.4f}".format(payload_out.quat_k)
    quat_real = "{:.4f}".format(payload_out.quat_real)
    accuracy = "{:d}".format(payload_out.accuracy)
    power_state = Responses(payload_out.power_state).name
    battery_voltage = "{:.4f}".format(payload_out.battery_voltage)
    battery_soc = "{:d}".format(payload_out.battery_soc)

    packet_table.add_row(['Out', timestamp, request, response, id_value, quat_i, quat_j, quat_k, quat_real, accuracy, power_state, battery_voltage, battery_soc])

    timestamp = "{:.4f}".format((current_time - prev_time)/1000)
    request = Requests(payload_in.request).name
    response = Responses(payload_in.response).name
    id_value = payload_in.id.decode().strip('_')
    quat_i = "{:.4f}".format(payload_in.quat_i)
    quat_j = "{:.4f}".format(payload_in.quat_j)
    quat_k = "{:.4f}".format(payload_in.quat_k)
    quat_real = "{:.4f}".format(payload_in.quat_real)
    accuracy = "{:d}".format(payload_in.accuracy)
    power_state = PowerStates(payload_in.power_state).name
    battery_voltage = "{:.4f}".format(payload_in.battery_voltage)
    battery_soc = "{:d}".format(payload_in.battery_soc)

    packet_table.add_row(['In', timestamp, request, response, id_value, quat_i, quat_j, quat_k, quat_real, accuracy, power_state, battery_voltage, battery_soc])

    print(colored(packet_table, 'white'))