import sys
import time
import serial
import csv
import re
import matplotlib.pyplot as plt

data_stream = serial.Serial('COM3', 115200)

current_time_list = []
vbatt_list = []


with open('test_output/test_output.csv', 'w', newline='') as test_output:
    writer = csv.writer(test_output)
    writer.writerow(["SampleNo.", "Timestamp(s)", "VBatt (mv)"])
    sampling_started = False

    while not sampling_started:
        # Read line, decode utf-8 style string, and strip trailing whitespace
        uart_payload = data_stream.readline().decode('utf-8').strip()
        # Remove ANSI escape codes
        uart_payload = re.sub(r'\x1b\[[0-9;]*m', '', uart_payload)
        # Split line into argument list with space as delimiter
        argument_list = uart_payload.split(" ")

        if (len(argument_list) >= 6):
            if (argument_list[5] == "SampleNo.:"):
                sampling_started = True

    for i in range(600):
        # Read line, decode utf-8 style string, and strip trailing whitespace
        uart_payload = data_stream.readline().decode('utf-8').strip()
        # Remove ANSI escape codes
        uart_payload = re.sub(r'\x1b\[[0-9;]*m', '', uart_payload)
        # Split line into argument list with space as delimiter
        argument_list = uart_payload.split(" ")


        # print sample info to console
        print(uart_payload)
        #calculate current time from timestamp
        current_time = float(argument_list[1].split("(")[1].split(")")[0]) * 0.001
        current_time_list.append(current_time)
        vbatt_list.append(argument_list[8])
        #write sample to CSV file
        writer.writerow([i + 1, current_time, argument_list[8]])

test_output.close()

#plot data
plt.scatter(current_time_list, vbatt_list)
plt.xlabel('Current Time (s)')
plt.ylabel('VBatt (mV)')
plt.title('Scatter Plot: Current Time vs. VBatt')
plt.show()

