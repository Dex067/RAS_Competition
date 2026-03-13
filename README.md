<!-- Not the ending,just the beginning -->

# RAS_Competition
- Not the ending, just the beginning
- I hope next time will be better
  
Arduino Smart Car
========================

A Multi-Mode Bluetooth-Controlled Robotic Car

Overview
--------
This project is an Arduino-based robotic vehicle that supports two main operating modes:

1.Manual Control Mode (Serial/Bluetooth Control)

2.Auto Line-Following Mode (Using 3 Digital IR Sensors)

Additional features include:

- Smooth non-blocking servo sweep using millis()

- Relay control for external devices (pump, LED strip, etc.)

- Spin and diagonal movement functions

- Software mode switching (Manual / Auto) using a Button on mobile.

-----------------------------------------------------------------------------------------------------------------

Hardware Components
-------------------
- Microcontroller: Arduino Uno (or Nano/Mega)
- Motor Driver: L298N (Dual H-Bridge)
- Motors: 2x DC Geared Motors (6V-12V)
- Bluetooth Module: HC-05 (UART)
- IR Sensors: 3x TCRT5000 (Line tracking)
- Servo Motor: SG90 (for ultrasonic panning)
- Power Supply: 12V Battery (or 3x 18650 Li-ion)
- Relay Module
-----------------------------------------------------------------------------------------------------------------

Operation Modes
--------------

1) Manual Mode

Activated when the 'M' command is pressed.

The robot receives commands via Serial (can be used with a Bluetooth module).

Available Commands

F → Move Forward

B → Move Backward

L → Turn Left

R → Turn Right

0 → Stop


P → Relay ON

O → Relay OFF

E → Start Servo Sweep

X → Stop Servo Sweep

Manual Mode Features

1:Full speed used for forward/backward movement

2:Half speed used for spins and diagonal moves

3:Instant motor stop

4:Non-blocking servo sweep control

------------------------------------------------------------------------------
 Line Following Mode ('M' Command) -----> it's the activated one by default.
-----

- Description:
  The car autonomously follows a dark line on a light surface (or vice versa) using IR reflectance sensors.

- Behavior Logic:
  A0 IR | A1 IR | A5 IR |     State      |
  ------|-------|-------|----------------|
  HIGH  | LOW   | LOW   | Move forward   |
  LOW   | HIGH  | LOW   | Move right     |
  LOW   | LOW   | HIGH  | Move left      |

else it spins in place.



This priority structure provides smoother and more stable tracking.

Servo Sweep System

1:Activated using command 'E'

2:Moves smoothly from 0° to 180° and back to 0°

3:Controlled using millis() (non-blocking timing)

4:Stops automatically after completing one sweep cycle

5:Can be stopped immediately using 'X'
	
- Key Features:
  - Lower speed setting (speed = 128) for precise control
  - 300ms turning delay to prevent oversteering
-----------------------------------------------------------------------------------------------------------------

Wiring Guide
-----------
Motor Connections:
- ENA (PWM) → Pin 3
- IN1 → Pin 7
- IN2 → Pin 8
- IN3 → Pin 12
- IN4 → Pin 13
- ENB (PWM) → Pin 5

Sensors & Actuators:
- Bluetooth (RX/TX) → Pins 0 & 1
- Servo → Pin 9
- IR Sensors → A1 (Right), A0 (Forward), A5 (Left)
- Relay → Pin 10
-----------------------------------------------------------------------------------------------------------------

Customization Options
--------------------
- Adjust Speeds: Modify the fullSpeed and halfSpeed values inside the code (255 and 128 by default).
- Tune Line Following: Adjust the sensor priority logic inside the autoControl() function for smoother tracking behavior.
- Modify Servo Sweep Speed: Change the interval value (currently 15 ms) to make the servo move faster or slower.
- Change Servo Range: Modify angle limits inside updateServo() if a different sweep range is required.
- Add New Manual Commands: Extend the switch(command) inside manualControl() to support additional movements or features.
-----------------------------------------------------------------------------------------------------------------

Bluetooth Apps (For Manual Control)
----------------------------------
- Android: "BlueDuino"
- iOS: "BlueTerm" or "BLE Terminal"
- Default Baud Rate: 9600
-----------------------------------------------------------------------------------------------------------------

Troubleshooting
--------------         
Issue                    |              Solution
-------------------------|-----------------------------------------
Motors not moving        |   Check L298N power supply and enable pins
Bluetooth not connecting |  Verify RX/TX crossover and baud rate
Line follower erratic    |  Adjust IR sensor height/angle
-----------------------------------------------------------------------------------------------------------------

