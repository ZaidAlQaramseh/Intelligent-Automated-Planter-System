# Intelligent-Automated-Planter-System
The Intelligent Automated Planter System project presents the design and integration of a tool that aims to improve and simplify indoor plants care by monitoring environmental measurements and automated irrigation. The Nucleo-L476RG, the core of the system, interfaces with several sensors and modules, such as temperature and humidity (DHT22), soil moisture, light (LDR), and a real time clock or RTC (DS3231). These sensors are used to record and measure environmental parameters that are crucial for plant growth. The system integrates a user-friendly interface, where the plant type is customized and it is also where warnings and measurements are shown to help the user provide optimal conditions using pre-specified thresholds. To automate the irrigation process, a water pump is used. The planter system is designed with affordability, energy efficiency, and ease of use in mind. 

## Hardware Used
- Nucleo-L476RG microcontroller  
- DHT22 for temperature and humidity  
- Analog soil moisture sensor  
- LDR (photoresistor) for light intensity  
- Nokia 5110 LCD (SPI-based display)  
- 4-way Joystick with push button  
- DS3231 RTC via I2C  
- Digital output-controlled water pump
