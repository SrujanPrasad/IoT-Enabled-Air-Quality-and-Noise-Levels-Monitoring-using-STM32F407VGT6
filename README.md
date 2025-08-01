# IoT-Enabled Air Quality and Noise Levels Monitoring using STM32

##  Problem Statement

Rapid urbanization and industrialization have led to an alarming increase in air and noise pollution levels, which pose serious threats to public health and the environment. Traditional environmental monitoring systems are often expensive, stationary, and lack real-time feedback. There is a critical need for a cost-effective, portable, and real-time solution to monitor air quality and noise pollution levels and alert users proactively.

## Objectives

- To design and implement an embedded system capable of monitoring air quality and noise levels in real time.
- To integrate cloud-based storage and visualization for remote monitoring.
- To provide instant alerts to users when pollution levels exceed predefined thresholds.
- To enable wireless communication using Bluetooth and Wi-Fi (ESP8266).
- To ensure low power consumption and portability for continuous deployment.

## Methodology

1. **Hardware Integration**:
   - Used **STM32F407VGT6** as the main microcontroller.
   - **MQ-135** gas sensor for detecting air pollutants (CO₂, NH₃, alcohol, benzene, smoke).
   - **KY-037** sound sensor for measuring noise levels.
   - **OLED Display** for real-time data visualization.
   - **Buzzer** for local alerts.
   - **ESP8266 Wi-Fi Module** for transmitting data to the cloud (ThingSpeak).
   - **Bluetooth module (optional)** for local wireless control/debugging.

2. **Software Design**:
   - STM32 HAL libraries are used for peripheral interfacing.
   - ADC polling and interrupt-based data acquisition for sensor inputs.
   - UART-based communication with ESP8266 for cloud updates.
   - Data pushed to **ThingSpeak** every 15 seconds via AT commands.
   - OLED updates using I²C interface.
   - Threshold-based buzzer alert system for abnormal values.

3. **Cloud Integration**:
   - ThingSpeak API used to store and visualize data trends.
   - Separate fields for air quality index and noise levels.
   - Public or private dashboard accessible remotely.

## Technologies Used

- **STM32F407VGT6** (ARM Cortex-M4)
- **MQ-135** Air Quality Sensor
- **KY-037** Microphone Sensor
- **OLED Display (I²C)**
- **ESP8266 Wi-Fi Module**
- **Buzzer**
- **MATLAB ThingSpeak IoT Cloud Platform**
- **STM32CubeIDE (Cube MX)**
- **Arduino Serial Monitor** (for ESP debug)

## Results 

![PXL_20240830_085255322 MP](https://github.com/user-attachments/assets/81e1e293-1ac9-4228-8872-2dd2f864726b) 

![PXL_20240830_085313497 MP](https://github.com/user-attachments/assets/a8dda5de-23fa-43f0-a1eb-d867dc9bad6b) 

![PXL_20240830_085335010 MP](https://github.com/user-attachments/assets/590261b7-8abf-49fc-849e-7c81f06098d1) 

![PXL_20240830_085336734 MP](https://github.com/user-attachments/assets/fae8077f-a458-4250-b0c9-67c9c3620273) 

![PXL_20250107_101925839](https://github.com/user-attachments/assets/ad98f862-2b50-4f16-9f09-61485e546820) 

<img width="1303" height="441" alt="image" src="https://github.com/user-attachments/assets/07231dba-9d24-430e-9fe8-2374e33c8e1b" />


## Key Outcomes

- Real-time monitoring of air and noise levels with on-device display.
- Successful transmission of environmental data to ThingSpeak every cycle.
- Cloud dashboard accessible remotely via browser or mobile.
- Instant audio alerts through the buzzer for critical pollution levels.
- Modular and extensible design for future environmental sensing additions.

## Future Scope

- Add a GPS module to geotag pollution data and create a live pollution map.
- Add data logging with SD card for offline analysis.
- Enable battery operation with solar charging for remote deployment.
- Integrate more sensors like temperature, humidity, and PM2.5.
- Use AI/ML for pollution trend prediction and automated threshold adaptation.
- Implement a mobile app for live monitoring and alerts.

## Conclusion

This project demonstrates a compact, efficient, and scalable IoT-based environmental monitoring system using STM32F407 and ESP8266.
It successfully captures and transmits air quality and noise data in real time, offering a valuable tool for public health monitoring and smart city applications. 
With minimal hardware and cloud integration, the system bridges the gap between embedded systems and IoT for real-world problem solving.
