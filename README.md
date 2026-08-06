# Table Tennis Referee

This is the Raspberry Pi code for the CC3501 product created by **Francesca Adcock, Keya Wong** and **Jamie White.**

The product is a **Table Tennis Referee Robot,** which automatically adjudicates casual and professional table tennis matches in order to avoid player disputes. The system automatically adjudicates ping pong matches by combining piezoelectric bounce sensing, camera-based ball tracking, Bluetooth communication, and automated scoring logic.

## System Overview

The project consists of two main computing devices:

### Devices

- **RP2040 Custom PCB** - Sensor processing, scoring logic, LED/Display control, servo control, sound generation, user input
- **Raspberry Pi** - Camera image processing, ball tracking, internet communication and ThingSpeak data upload

Communication between the RP2040 and Raspberry Pi is performed using Bluetooth (RN4870 modules).

## Key Features

 - Piezoelectric sensors that detect ball bounces and net hits
 - Camera tracking to cross-reference ball bounces, and used to detect which side of the table the ball is currently located on
 - Internet communication through Thingspeak to upload current scores, total wins in a current session and all-time wins
 - Bluetooth connection between the RP2040 custom board inside the robot and the Raspberry Pi which does the image processing and internet communication
 - Servo motor which moves the robot's arms depending on certain events during a game
 - LEDs to indicate points, serves and faults
 - 4-digit 7-segment display to show player scores, and communicate lets and game modes
 - Buttons to manually adjust player scores
 - Different game modes for casual and professional-style games with different rules and regulations, and ability to run games with and without camera included
 - Automatic mode sensing by placing different hats on the head of the robot
 - Sounds played through the piezo electric sensors (which can act as a buzzer) to indicate when points are scored, who serves, end of the match, button selection and faults
 - Portability of the robot by powering it with a 9V battery
 - Power protection and decision making if powered by both micro-USB and Battery, to prioritize USB power and avoid reverse current
 - Current limiting and motor circuit protection both in hardware and software to prevent overcurrent when motor stalls
 - Testing to ensure all systems work as appropriate


## Hardware Requirements
### RP2040 Board
- RP2040 microcontroller and board
- RN4870 Bluetooth module
- HT16K33 4-dgiti 7-segment display
- WS2812 LEDs
- Piezoelectric sensors
- Servo Motor
- Push buttons
- Hats for modes
- Robot
- 9V battery input (or microUSB power)

### Raspberry Pi
- Raspberry Pi 4
- Raspberry Pi Camera Module
- RN4870 Bluetooth module
- Internet connection for ThingSpeak uploads

## Software Requirements
### RP2040 Firmware
- Raspberry Pi Pico SDK
- CMake
- ARM GCC toolchain

### Raspberry Pi
- C++17
- OpenCV
- GStreamer
- CMake

## Game Modes
- **Casual** - Simplified scoring rules for recreational play, such as no punishment for lets
- **Professional Long** - Strict rules, games to 21 points
- **Professional Short** - Strict rules, games to 11 points
- **No Sound** - Sounds and motor movements turned off during the game, for smoothest experience
- **N/A** - Default when no hat connected

## Known Limitations
- Piezo sensors require a short settling period after playing a sound before accurate bounce detection resumes
- Camera tracking performance depends on lighting conditions and camera placement
- Bluetooth modules cannot enter command mode while connected, requiring a controlled connection test and disconnect procedure
- Motor movements can cause vibrations that set off the piezoelectric sensors

## Future Improvements
- Dedicated buzzer instead of reusing piezo sensors for both sensing and audio output
- Extra sensors to detect ball bounces and differentiate from accidental table touches
- Video replaybility for epic rallies and wins

## Authors
- Francesca Adcock
- Keya Wong
- Jamie White

Developed for CC3501 Engineering Design Project - James Cook University
