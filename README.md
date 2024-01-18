<a name="readme-top"></a>

## About
ADC data logging firmware and python script for battery testing. 

## How to Use
- Update your esp-idf to V5.2
- Build and flash the firmware in the Firmware/adc_data_grabber 
- Edit the PythonDataLogger/AdcDataLogger.py to reflect the COM port your esp32 is using
- Execute the python script (using cmd prompt in PythonDataLogger: "python AdcDataLogger.py")
- If the above step doesn't work, pip install any missing packages (pyserial, matplotlib)

# Output 
The python script will store any collected samples within PythonDataLogger/test_output.csv, it will also generate a plot of the samples with matplotlib. 

<p align="right">(<a href="#readme-top">back to top</a>)</p>
