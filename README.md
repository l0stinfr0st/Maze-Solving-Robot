\# Maze Solving Robot



A Bluetooth-controlled autonomous robot capable of navigating and solving physical mazes using ultrasonic sensing and path memory. The robot stores the discovered route in EEPROM and plays a unique tune representing the final path once the maze is completed.



\## Features



\- Manual control through Bluetooth and a mobile phone

\- Autonomous maze solving

\- Path storage using Arduino EEPROM

\- Audio feedback based on the discovered path

\- Obstacle detection using an ultrasonic sensor

\- Servo-mounted sensor scanning for directional decisions

\- Light sensor interrupt to detect maze completion



\## Hardware Used



\- Arduino

\- L298N motor driver

\- HC-SR04 ultrasonic sensor

\- Servo motor

\- Bluetooth module

\- Light sensor

\- Buzzer

\- DC motors and chassis



\## How It Works



The robot continuously scans its surroundings using a servo-mounted ultrasonic sensor. It checks the left, front, and right directions to determine the best movement path through the maze.



Once the maze is completed, a light sensor triggers an interrupt to signal completion. The robot then:

\- Stops movement

\- Stores the discovered route in EEPROM

\- Plays a tune corresponding to the path taken



The robot can also be manually controlled through a Bluetooth connection using a mobile phone.



\## Maze Solving Logic



The navigation logic follows this priority:



1\. Check left path

2\. If blocked, check front

3\. If blocked again, check right

4\. Perform turns by rotating wheels in opposite directions



The robot also moves slightly forward after each turn for alignment.



\## EEPROM Path Storage



The discovered maze path is stored in EEPROM so the route can persist even after power loss. This allows replaying or analyzing the solved path later.



\## Future Improvements



\- Faster shortest-path optimization

\- PID motor control

\- Improved mapping and localization

\- Mobile app interface

\- Real-time maze visualization



\## Author



Developed by Ahmad Ghazi as part of an embedded systems and robotics project.

