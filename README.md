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
- Decide on full breadth of features 
- Finalize OLED selection
- Determine whether an external peripheral or onboard peripheral will be used for communication with software 
- Finalize IMU selection ✔️
- Determine user inputs (most likely single navigation switch) ✔️

### Software
- Decide on choice of language
- Decide on choice of graphics rendering or plotting library
- Decide on choice of GUI library
- Write backend for communication with firmware (for prototyping UART/serial can be used)
- Attempt basic plotting ✔️ 
 
### Firmware

- Run tests and determine whether WiFi or BLE will be used to communicate with software
- Write driver for BLE or UDP (maybe TCP?) over WiFi
- Write battery gauge driver
- Write suit calibration routine 
- Run tests with BNO-055 IMU to determine if esp-idf I2C driver is suitable for comms ✔️
- Write driver class for SPI (implementation of BNO085 later in project) ✔️
- Implement BNO085 sensor ✔️
- Write menu system for OLED ✔️
- Write drivers for rocker switch or choice of controls ✔️

## License
Distributed under the GNU 3.0 License. See `LICENSE.md` for more information.
<p align="right">(<a href="#readme-top">back to top</a>)</p>
