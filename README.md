# Smart Gas Leakage Detector with Fire Extinguisher System

## Project Overview
This project presents the design and implementation of a smart gas leakage detection and fire suppression system for domestic safety.

The system is designed to detect gas leakage, identify fire outbreaks and provide timely alerts to occupants. It uses different sensors to monitor gas concentration, flame and temperature. When danger is detected, the system activates an alarm and sends SMS notifications to the user. It also activates an automatic water sprinkler system when the required fire detection conditions are met.

## Objectives
- To detect gas leakage in domestic environments.
- To detect fire outbreaks using flame and temperature sensors.
- To provide audible warnings during emergencies.
- To send SMS alerts through a GSM module.
- To activate an automatic fire suppression system.

## Components Used
- Arduino Uno
- MQ-2 Gas Sensor
- MQ-5 Gas Sensor
- MQ-6 Gas Sensor
- IR Flame Sensor
- DS18B20 Temperature Sensor
- SIM800L GSM Module
- 16×2 I2C LCD
- 4×4 Keypad
- Buzzer
- Relay Module
- Water Pump and Sprinkler

## How the System Works
The gas sensors monitor the surrounding environment for gas leakage. When gas concentration reaches the programmed threshold, the buzzer is activated and an SMS alert is sent through the GSM module.

The flame sensor and temperature sensor work together to identify fire conditions. When both required conditions are satisfied, the system activates the water pump and sprinkler to provide an automatic response.

The LCD displays relevant system information.

## Technologies Used
- Arduino Programming (C/C++)
- Embedded Systems
- Gas and Flame Detection
- GSM Communication
- Automatic Fire Suppression

## Project Purpose
The project aims to improve domestic safety by providing an affordable system for early gas leakage detection, fire warning and automatic response.
