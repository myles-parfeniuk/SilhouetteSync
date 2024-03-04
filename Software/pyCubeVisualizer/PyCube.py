import socket
import keyboard
from ctypes import *
from enum import Enum
import colorama
from colorama import init
from termcolor import colored
from prettytable import PrettyTable
import numpy as np
import pygame

from pygame.locals import *

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

class Payload(Structure):
    _fields_ = [("request", c_uint8),
                ("response", c_uint8),
                ("id", c_uint8),
                ("quat_i", c_float),
                ("quat_j", c_float),
                ("quat_k", c_float),
                ("quat_real", c_float),
                ("accuracy", c_uint8),
                ("time_stamp", c_uint64)]

server_address_1 = ('192.168.1.81', 49160)
data_stream = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
data_stream.settimeout(0.015)

init()

# Initialize Pygame
pygame.init()
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600
screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
clock = pygame.time.Clock()

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

prev_time = 0

def draw_cube(rotation_matrix):
    vertices = [
        np.array([-1, -1, -1]),
        np.array([1, -1, -1]),
        np.array([1, 1, -1]),
        np.array([-1, 1, -1]),
        np.array([-1, -1, 1]),
        np.array([1, -1, 1]),
        np.array([1, 1, 1]),
        np.array([-1, 1, 1])
    ]
    edges = [
        (0, 1), (1, 2), (2, 3), (3, 0),
        (4, 5), (5, 6), (6, 7), (7, 4),
        (0, 4), (1, 5), (2, 6), (3, 7)
    ]
    rotated_vertices = [np.dot(rotation_matrix, v) for v in vertices]
    points = []
    for v in rotated_vertices:
        x = int(v[0] * SCREEN_HEIGHT / (v[2] + 6) + SCREEN_WIDTH / 2)
        y = int(v[1] * SCREEN_HEIGHT / (v[2] + 6) + SCREEN_HEIGHT / 2)
        points.append((x, y))
    for edge in edges:
        pygame.draw.line(screen, (255, 255, 255), points[edge[0]], points[edge[1]], 2)

while True:
    try:
        payload_out = send_packet(Requests.SAMPLE_REQ.value, Responses.NULL_RESP.value, server_address_1)
        payload_in = receive_packet()

        print_packet(payload_in, payload_out, payload_in.time_stamp, prev_time)
        prev_time = payload_in.time_stamp

        # Convert quaternion to rotation matrix
        qw, qx, qy, qz = payload_in.quat_real, payload_in.quat_i, payload_in.quat_j, payload_in.quat_k
        rotation_matrix = np.array([
            [1 - 2*qy**2 - 2*qz**2, 2*qx*qy - 2*qz*qw, 2*qx*qz + 2*qy*qw],
            [2*qx*qy + 2*qz*qw, 1 - 2*qx**2 - 2*qz**2, 2*qy*qz - 2*qx*qw],
            [2*qx*qz - 2*qy*qw, 2*qy*qz + 2*qx*qw, 1 - 2*qx**2 - 2*qy**2]
        ])

        # Clear the screen
        screen.fill((0, 0, 0))

        # Draw a cube using the rotation matrix
        draw_cube(rotation_matrix)

        # Update the display
        pygame.display.flip()

    except socket.timeout:
        print(colored('ERROR: Timeout occurred, resending request...', 'red'))
        continue

    if keyboard.is_pressed('q'):
        print(colored('Quitting, socket shutting down.', 'magenta'))
        data_stream.close()
        pygame.quit()
        break