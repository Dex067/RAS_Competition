// Not the ending, just the beginning

/*
 * @file   RobotControlSystem.ino
 * @brief  Arduino robot control system with auto line-following, manual serial control, servo sweep, and pump control
 * @author Mohamed Hassan
 * @date   2026-03-06
 *
 * @details
 * This project implements a motor-driven robot with:
 * - Automatic line-following using three IR sensors
 * - Manual control via serial commands
 * - Servo sweep for scanning (0-180°)
 * - Relay-controlled pump
 * The system can switch between automatic and manual modes during runtime.
 *
 * Features:
 * - Motor movement: forward, backward, left/right turn, spin in place, stop
 * - Servo control: automatic sweep or specific angle
 * - Pump control via relay
 * - Serial-based manual commands
 * - Mode toggle between auto and manual operation
 *
 * @note
 * Non-blocking servo sweep using millis() allows simultaneous motor control.
 * Mode toggle includes debounce to prevent accidental switching.
 */

#include <Servo.h>
#include <ctype.h>

//=================== PIN DEFINITIONS ===================//
// Motor driver pins
#define IN1 7
#define IN2 8
#define IN3 12
#define IN4 13
#define EN1 3
#define EN2 5

// IR line follower sensors
#define rightSensor A1
#define midsensor A0
#define leftSensor A5
#define white LOW
#define black HIGH

// Pump relay pin
#define RELAY_PIN 10

//=================== SERVO CONFIG ===================//
Servo myservo;
bool servoActive = false;         // Flag to indicate automatic servo sweep
bool servoForward = true;         // Current sweep direction
int servoAngle = 0;               // Current servo angle
unsigned long previousMillis = 0; // Timer for sweep intervals
const int interval = 50;          // Servo sweep step interval (ms)
int lastServoAngle = -1;          // Last angle sent to servo (to prevent unnecessary writes)

//=================== SERIAL CONTROL ===================//
char command = 0;                 // Current serial command
bool isAuto = true;               // Default: auto line-following mode
unsigned long lastModeSwitch = 0; // Timestamp of last mode toggle (debouncing)

//=================== MOTOR SPEED ===================//
const int fullspeed = 255; // Maximum motor speed
const int halfspeed = 128; // Half speed (used for turns)

//=================== PUMP STATE ===================//
bool pumpState = false; // Tracks current pump state

//=================== FUNCTION DEFINITIONS ===================//

/**
 * @brief Set the pump state via relay
 * @param state True = ON, False = OFF
 */
void setPump(bool state) {
  pumpState = state;
  digitalWrite(RELAY_PIN, state ? HIGH : LOW);
}

/**
 * @brief Move robot forward
 */
void moveForward() {
  analogWrite(EN1, fullspeed);
  analogWrite(EN2, fullspeed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

/**
 * @brief Move robot backward
 */
void moveBackward() {
  analogWrite(EN1, fullspeed);
  analogWrite(EN2, fullspeed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

/**
 * @brief Turn robot left (right motor forward, left motor stopped)
 */
void turnLeft() {
  analogWrite(EN1, fullspeed);
  analogWrite(EN2, fullspeed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

/**
 * @brief Turn robot right (left motor forward, right motor stopped)
 */
void turnRight() {
  analogWrite(EN1, fullspeed);
  analogWrite(EN2, fullspeed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

/**
 * @brief Spin robot in place (left motor forward, right motor backward)
 */
void spininplace() {
  analogWrite(EN1, halfspeed);
  analogWrite(EN2, halfspeed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

/**
 * @brief Stop all motors immediately
 */
void stopMotors() {
  analogWrite(EN1, 0);
  analogWrite(EN2, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

//=================== SERVO FUNCTIONS ===================//

/**
 * @brief Start automatic sweeping of the servo (0-180°)
 */
void startServoSweep() {
  servoActive = true;
}

/**
 * @brief Update servo sweep position
 * Should be called regularly in loop()
 */
void updateServo() {
  if (!servoActive) return;

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (servoForward) {
      servoAngle++;
      if (servoAngle >= 180) servoForward = false;
    } else {
      servoAngle--;
      if (servoAngle <= 0) servoForward = true;
    }

    // Only update servo if angle changed
    if (servoAngle != lastServoAngle) {
      myservo.write(servoAngle);
      lastServoAngle = servoAngle;
    }
  }
}

/**
 * @brief Set servo to a specific angle and stop sweep
 * @param angle Desired angle (0-180)
 */
void setServoAngle(int angle) {
  if (angle >= 0 && angle <= 180) {
    servoAngle = angle;
    myservo.write(angle);
    servoActive = false;
  }
}

//=================== MODE TOGGLE ===================//

/**
 * @brief Toggle between automatic and manual control modes
 * @note Stops motors immediately when switching
 */
void toggleMode() {
  isAuto = !isAuto;
  stopMotors();

  if (isAuto) Serial.println("Mode: AUTO");
  else Serial.println("Mode: MANUAL");
}

//=================== MANUAL CONTROL ===================//

/**
 * @brief Execute robot actions based on serial command
 * Commands:
 * F = Forward, B = Backward, L = Turn Left, R = Turn Right
 * 0 = Stop, E = Start Servo Sweep, X = Stop Servo Sweep
 * P = Pump ON, O = Pump OFF, M = Toggle Auto/Manual
 */
void manualControl() {
  switch (command) {
    case 'F': moveForward(); break;
    case 'B': moveBackward(); break;
    case 'L': turnLeft(); break;
    case 'R': turnRight(); break;
    case '0': stopMotors(); break;
    case 'E': startServoSweep(); break;
    case 'X': servoActive = false; break;
    case 'P': setPump(true); break;
    case 'O': setPump(false); break;
  }

  command = 0; // Prevent repeated execution
}

//=================== AUTOMATIC LINE FOLLOWING ===================//

/**
 * @brief Control motors based on IR sensor input
 * @note Uses three sensors: left, mid, right
 */
void autoControl() {
  int left = digitalRead(leftSensor);
  int mid = digitalRead(midsensor);
  int right = digitalRead(rightSensor);

  if (left == white && mid == black && right == white) moveForward();
  else if (left == black && mid == white && right == white) turnLeft();
  else if (left == white && mid == white && right == black) turnRight();
  else if (left == black && mid == black && right == black) stopMotors();
  else spininplace(); // Search for line if lost
}

//=================== SERIAL INPUT ===================//

/**
 * @brief Read and process serial commands
 * Includes mode toggle with debounce
 */
void readSerialCommand() {
  if (Serial.available() > 0) {
    command = Serial.read();
    command = toupper(command);

    if (command == '\n' || command == '\r') return;

    if (command == 'M' && millis() - lastModeSwitch > 300) {
      toggleMode();
      lastModeSwitch = millis();
    }
  }
}

//=================== SETUP ===================//
void setup() {
  // Motor pins
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(EN1, OUTPUT); pinMode(EN2, OUTPUT);

  // IR sensors
  pinMode(leftSensor, INPUT);
  pinMode(midsensor, INPUT);
  pinMode(rightSensor, INPUT);

  // Pump relay
  pinMode(RELAY_PIN, OUTPUT);
  setPump(false);

  // Serial
  Serial.begin(9600);

  // Servo
  myservo.attach(9);
  myservo.write(0);

  Serial.println("System Initialized. Mode: AUTO");
}

//=================== MAIN LOOP ===================//
// Main loop: check serial input, run auto/manual control, update servo sweep
void loop() {
  readSerialCommand();

  if (isAuto) autoControl();
  else manualControl();

  updateServo();
}