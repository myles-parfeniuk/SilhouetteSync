import pygame
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
from numpy import array
from math import cos, sin
import serial
import re
import sys

 """
    Script that reads live data from IMU and draws a cube according to the received absololute heading.
 """

X, Y, Z = 0, 1, 2

def rotation_matrix(alpha, beta, gamma):
    """
    rotation matrix of alpha (roll), beta (pitch), gamma (yaw) in radians around x, y, z axes (respectively)
    """
    s_alpha, c_alpha = sin(alpha), cos(alpha)
    s_beta, c_beta = sin(beta), cos(beta)
    s_gamma, c_gamma = sin(gamma), cos(gamma)
    return [
        [c_beta*c_gamma, -c_beta*s_gamma, s_beta],
        [c_alpha*s_gamma + s_alpha*s_beta*c_gamma, c_alpha*c_gamma - s_gamma*s_alpha*s_beta, -c_beta*s_alpha],
        [s_gamma*s_alpha - c_alpha*s_beta*c_gamma, c_alpha*s_gamma*s_beta + s_alpha*c_gamma, c_alpha*c_beta]
    ]


class Physical:
    def __init__(self, vertices, edges):
        """
        a 3D object that can rotate around the three axes
        :param vertices: a tuple of points (each has 3 coordinates)
        :param edges: a tuple of pairs (each pair is a set containing 2 vertices' indexes)
        """
        self.__vertices = array(vertices)
        self.__edges = tuple(edges)
        self.__rotation = [0, 0, 0]  # radians around each axis

    def rotate(self, r_vec):
        self.__rotation[0] = r_vec[0]
        self.__rotation[1] = r_vec[1]
        self.__rotation[2] = r_vec[2]

    @property
    def lines(self):
        location = self.__vertices.dot(rotation_matrix(self.__rotation[0],  self.__rotation[1],  self.__rotation[2]))        # an index->location mapping
        return [(location[v1], location[v2]) for v1, v2 in self.__edges]


BLACK, RED = (0, 0, 0), (255, 128, 128)


class Paint:
    def __init__(self, shape):
        self.__shape = shape
        self.__size = 450, 450
        self.__clock = pygame.time.Clock()
        pygame.init()
        pygame.font.init()
        pygame.display.set_mode(self.__size, pygame.DOUBLEBUF | pygame.OPENGL)
        gluPerspective(45, (self.__size[0] / self.__size[1]), 0.1, 50.0)
        glTranslatef(0.0, 0.0, -5)

        self.__mainloop()

    def __handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                exit()

    def __draw_shape(self, rotation, velocity):
        # Draw a red point at the center of the cube
        glColor3f(1.0, 0.0, 0.0)  # Red color
        glPointSize(5.0)  # Adjust point size if needed
        glBegin(GL_POINTS)
        glVertex3f(0.0, 0.0, 0.0)  # Center of the cube
        glEnd()

        # Get the current rotation matrix of the cube
        rotation_matrix_cube = rotation_matrix(*rotation)
        self.__shape.rotate(rotation)
        # Draw axes markers protruding from the red point with cube's rotation
        axes_length = 0.25  # Length of axes markers (adjust as needed)
        glBegin(GL_LINES)

        # X-axis (Red)
        glColor3f(1.0, 0.0, 0.0)
        glVertex3f(0.0, 0.0, 0.0)
        glVertex3fv(array([axes_length, 0.0, 0.0]).dot(rotation_matrix_cube))

        # Y-axis (Green)
        glColor3f(0.0, 1.0, 0.0)
        glVertex3f(0.0, 0.0, 0.0)
        glVertex3fv(array([0.0, axes_length, 0.0]).dot(rotation_matrix_cube))

        # Z-axis (Blue)
        glColor3f(0.0, 0.0, 1.0)
        glVertex3f(0.0, 0.0, 0.0)
        glVertex3fv(array([0.0, 0.0, axes_length]).dot(rotation_matrix_cube))

        glEnd()

        glColor3f(0.0, 1.0, 0.0)  # Bright green
        glBegin(GL_LINES)
        for start, end in self.__shape.lines:
            glVertex3fv(start)
            glVertex3fv(end)
        glEnd()

        velocity_disp = "Velocity (Rad/s) X: %.3f Y: %.3f Z: %.3f" % (velocity[0], velocity[1], float(velocity[2]))


    def __mainloop(self):
        rotation = [0, 0, 0]
        velocity = [0, 0, 0]
        # open comport
        data_stream = serial.Serial('COM4', 115200)
        while True:

            # Read line, decode utf-8 style string, and strip trailing whitespace
            uart_payload = data_stream.readline().decode('utf-8').strip()

            # Remove ANSI escape codes
            uart_payload = re.sub(r'\x1b\[[0-9;]*m', '', uart_payload)

            # Split line into argument list with space as delimiter
            argument_list = uart_payload.split(" ")

            if argument_list[2] == "GRV:":
                rotation = [float(argument_list[3]), float(argument_list[4]), float(argument_list[5])]
                print(rotation)

            elif argument_list[2] == "VEL:":
                velocity = [float(argument_list[3]), float(argument_list[4]), (argument_list[5])]


            self.__handle_events()
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
            self.__draw_shape(rotation, velocity)
            pygame.display.flip()
            pygame.time.wait(10)
            self.__clock.tick(40)


def main():
    from pygame import K_q, K_w, K_a, K_s, K_z, K_x

    cube = Physical(
        vertices=((1, 1, 1), (1, 1, -1), (1, -1, 1), (1, -1, -1), (-1, 1, 1), (-1, 1, -1), (-1, -1, 1), (-1, -1, -1)),
        edges=({0, 1}, {0, 2}, {2, 3}, {1, 3}, {4, 5}, {4, 6}, {6, 7}, {5, 7}, {0, 4}, {1, 5}, {2, 6}, {3, 7})
    )


    pygame.display.set_caption('IMU Cube Visualizer')
    Paint(cube)

if __name__ == '__main__':
    main()