<a name="readme-top"></a>
![image](SillhoutteSync_banner.png)
<ol>
  <li><a href="#about">About</a></li>
  <li><a href="#to-do">To Do</a>
    <ul>
      <li><a href="#hardware">Hardware</a></li>
      <li><a href="#software">Software</a></li>
      <li><a href="#firmware">Firmware</a></li>
    </ul>
  </li>
  <li><a href="#license">License</a></li>
</ol>

## About
Open source motion tracker band intended for athletics analytics.

## To Do

### Hardware
- Decide on full breadth of features ✔️
- Finalize IMU selection ✔️
- Determine user inputs ✔️
- Finalize schematic ✔️
- Finalize layout✔️
- First PCB bring-up
- Test power draw for battery selection
- Design device enclosure

### Software
- Attempt basic plotting ✔️ 
- Decide on choice of language ✔️
- Decide on choice of graphics rendering or plotting library ✔️
- Test animation and basic app layout ✔️
- Write backend for communication with firmware (for prototyping UART/serial can be used)
- Algorithm to derive individual joint angle regardless of absolute body position
- Algorithm to follow through workout and report on effectiveness
- Tracker information screen
 
### Firmware
- Run tests and determine whether WiFi or BLE will be used to communicate with software ✔️
- Write driver for BLE or UDP (maybe TCP?) over WiFi ✔️
- Write driver class for BNO085 SPI IMU ✔️
- Implement addressable LEDs ✔️
- Write battery gauge driver (decided to go with ADC battery measurement)
- Write suit calibration routine 
- Improve wireless stability
- Implement device power up and power sensing algorithm

## License
Distributed under the GNU 3.0 License. See `LICENSE.md` for more information.
<p align="right">(<a href="#readme-top">back to top</a>)</p>
