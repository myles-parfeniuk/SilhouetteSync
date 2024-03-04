import pygame
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
from numpy import array, dot
import socket
from ctypes import Structure, c_uint8, c_uint64, c_float
from enum import Enum
import re

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

# Define a Payload structure for UDP communication
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

# UDP socket configuration
server_address = ('192.168.1.81', 49160)
data_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
data_socket.settimeout(0.015)  # 15 milliseconds

# OpenGL cube configuration
class Physical:
    def __init__(self, vertices, edges):
        self.__vertices = array(vertices)
        self.__edges = tuple(edges)
        self.__rotation = [1.0, 0.0, 0.0, 0.0]  # Initial quaternion

    def rotate(self, quat):
        self.__rotation = quat

    @property
    def lines(self):
        location = self.__vertices.dot(rotation_matrix_quaternion(self.__rotation))
        return [(location[v1], location[v2]) for v1, v2 in self.__edges]

def rotation_matrix_quaternion(quat):
    """
    Generate a rotation matrix from a quaternion
    """
    q0, q1, q2, q3 = quat
    return [
        [1 - 2*q2**2 - 2*q3**2, 2*q1*q2 - 2*q0*q3, 2*q1*q3 + 2*q0*q2],
        [2*q1*q2 + 2*q0*q3, 1 - 2*q1**2 - 2*q3**2, 2*q2*q3 - 2*q0*q1],
        [2*q1*q3 - 2*q0*q2, 2*q2*q3 + 2*q0*q1, 1 - 2*q1**2 - 2*q2**2]
    ]

def receive_quaternion():
    try:
        payload_in = Payload(0, 0)
        buff, _ = data_socket.recvfrom(sizeof(Payload))
        if len(buff) != sizeof(Payload):
            print('ERROR: Received buffer size does not match expected payload size.')
        payload_in = Payload.from_buffer_copy(buff)
        return payload_in.quat_real, payload_in.quat_i, payload_in.quat_j, payload_in.quat_k
    except socket.timeout:
        print('ERROR: Timeout occurred in recvfrom. Unable to receive quaternion.')
        return 0, 0, 0, 0

def draw_shape(shape):
    try:
        rotation_matrix_cube = rotation_matrix_quaternion(shape._Physical__rotation)
        shape.rotate(receive_quaternion())  # Update the rotation of the cube using received quaternion
        glColor3f(0.0, 1.0, 0.0)  # Bright green
        glBegin(GL_LINES)
        for start, end in shape.lines:
            glVertex3fv(dot(start, rotation_matrix_cube))
            glVertex3fv(dot(end, rotation_matrix_cube))
        glEnd()
    except socket.timeout:
        print('ERROR: Timeout occurred in recvfrom. Unable to draw shape.')

def main():
    cube = Physical(
        vertices=((1, 1, 1), (1, 1, -1), (1, -1, 1), (1, -1, -1), (-1, 1, 1), (-1, 1, -1), (-1, -1, 1), (-1, -1, -1)),
        edges=({0, 1}, {0, 2}, {2, 3}, {1, 3}, {4, 5}, {4, 6}, {6, 7}, {5, 7}, {0, 4}, {1, 5}, {2, 6}, {3, 7})
    )

    pygame.init()
    pygame.display.set_mode((800, 600), pygame.DOUBLEBUF | pygame.OPENGL)
    gluPerspective(45, (800 / 600), 0.1, 50.0)
    glTranslatef(0.0, 0.0, -5)

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                return

