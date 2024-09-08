# Smart_Green_House

Smart Green House implimentation using the stm2 as slaves to the modbus connect to the cloud via gateway (rugged board A5d2x).

    PROJECT OVERVIEW

Introduction:
The SMART GREEN HOUSE project is designed to revolutionise the way we manage and control greenhouse environments. By integrating advanced microcontroller technology with modern cloud-based solutions, this project aims to create a sophisticated system for monitoring and managing greenhouse conditions with ease and precision.

In a traditional greenhouse, managing environmental factors such as temperature and humidity can be challenging, especially when aiming to optimise conditions for plant growth and overall productivity. The SMART GREEN HOUSE project addresses these challenges by utilising a combination of two STM32 F446RE microcontrollers and a Rugged Board A5D2X, orchestrated to provide a seamless and automated control system.

By leveraging these advanced technologies, the SMART GREEN HOUSE project represents a significant step forward in greenhouse automation, providing a powerful tool for optimising environmental conditions and enhancing agricultural productivity.

HARDWARE  
SPECIFICATIONS

1. System Components:
   ●Rugged Board A5D2X (Master Controller)
   ○Processor: High-performance SOM with sufficient processing power for handling Modbus communication and control tasks.
   ○Communication Ports: Multiple I/O ports for Modbus communication and connectivity with STM32 microcontrollers.
   ○Power Supply: 12V DC or as per the Rugged Board specifications.
   ○Connectivity: Support for network communication (e.g., Ethernet or Wi-Fi) for integration with PhyCloud and remote control.
   ○Operating System: Compatible with the operating system or firmware for handling Modbus protocol and PhyCloud integration.
   ●STM32 F446RE Microcontrollers (Slave Controllers)
   ○Processor: STM32F446RE microcontroller with ARM Cortex-M4 core.
   ○Clock Speed: Up to 180 MHz.
   ○Flash Memory: 512 KB.
   ○RAM: 128 KB.
   ○Communication Interfaces: UART, SPI, I2C, and GPIO for sensor data acquisition and Modbus communication.
   ○Analog-to-Digital Converter (ADC): For reading sensor data.
   ○Power Supply: 3.3V or 5V DC as per STM32 specifications

●Sensors and Actuators
Temperature Sensor:
KY-013 analog temperature sensor, suitable for greenhouse environments.
●Temperature Range: -55°C to +125°C.
●Accuracy: ±0.5°C.
●
○Relay Module: For controlling greenhouse devices (e.g., irrigation, fans, heaters).
■Relay Type: SPST (Single Pole Single Throw) or as required.
■Relay Rating: Suitable for the voltage and current requirements of the greenhouse devices. 2. Communication Protocol:
●Modbus: RTU or TCP for communication between the Rugged Board and STM32 microcontrollers.
○Baud Rate: Configurable (commonly 9600, 19200, or 115200 bps).
○Data Bits: 8.
○Parity: None.
○Stop Bits: 1 or 2. 3. Mobile App Integration:
●PhyCloud Platform: For remote monitoring and control.
○Connectivity: Integration with PhyCloud through Internet connectivity (e.g., Wi-Fi, Ethernet).
○Mobile App: Compatible with Android and iOS devices.
○Features: Real-time data visualization, remote control, notifications.

4. Power Supply and Requirements:
   ●Overall Power Supply: Ensure a stable power source for the entire system.
   ●Rugged Board: Typically 12V DC or as specified by the manufacturer.
   ●STM32 Microcontrollers: Powered by 3.3V or 5V DC.
   ●Power Consumption: To be calculated based on the actual hardware configuration and connected devices.

5. Environmental Requirements:
   ●Operating Temperature: 0°C to 50°C (for indoor greenhouse environments).
   ●Humidity: Suitable for operation in a high-humidity environment.

6. Enclosures and Mounting:
   ●Enclosures: Weatherproof or protected enclosures for outdoor components to ensure durability.
   ●Mounting: Secure mounting for microcontrollers, sensors, and relays within the greenhouse.a
   ●
   Modbus RTU (Remote Terminal Unit) Frame Structure:
   ●Slave Address: Identifies which slave device the master is communicating with. In this project, each STM32 slave has a unique Modbus address.
   ●Function Code: Indicates the operation to be performed. For example, reading sensor data (function code 03) or writing to a relay (function code 05).
   ●Data: Contains the payload of the message, such as the temperature reading or the command to control the relay.
   ●CRC (Cyclic Redundancy Check): A checksum used for error checking to ensure data integrity during transmission.
   Function Codes Used:
   ●Read Holding Registers (Function Code 03): The master uses this function code to read the temperature data from Slave 2. The sensor data is typically stored in one or more holding registers.the function code 3 will be implemented on slave 2
   ●Write Single Coil (Function Code 05): The master uses this function code to control the relay connected to Slave 1. This function writes a single bit to turn the relay on or off.The function code 5 is implemented on slave 1

7. Software and Firmware:
   ●Firmware: Custom firmware for STM32 microcontrollers for sensor data acquisition, Modbus communication, and relay control.
   ●Software: Configuration and control software for the Rugged Board, including simple Modbus management and PhyCloud integration.
8. Safety and Compliance:
   ●Standards: Compliance with relevant electrical and safety standards.
   ●Certifications: Ensure components and system meet required certifications for safety and reliability.
   ●9. Documentation:
   These specifications outline the key hardware and system requirements for the SMART GREEN HOUSE project, ensuring that all components work together effectively to achieve the desired automation and control capabilities.

9. virtual Modbus master software:
    .Simply modbus Master: https://www.simplymodbus.ca/RTUmaster.htm
    .Modpoll Modbus Master : https://www.modbustools.com/download.html
   
