#include <Arduino.h>

#include <LiquidCrystal.h>
#include <AccelStepper.h>

// Define pins for sensors, buttons, and output devices
#define PHOTO_SENSOR_PIN A0      // Photo sensor input pin
#define ROCK_SENSOR_PIN A1       // Rock sensor input pin
#define START_BUTTON_PIN 22      // Start button input pin
#define STOP_BUTTON_PIN 23       // Stop button input pin
#define EMERGENCY_BUTTON_PIN 24  // Emergency button input pin
#define MOTOR_RELAY_PIN 25       // Motor relay control pin
#define BUZZER_PIN 26            // Buzzer output pin
#define LED_ALERT_PIN 27         // LED alert pin

// Stepper motor pin definitions for two motors
#define INT1_M1 17
#define INT2_M1 16
#define INT3_M1 7
#define INT4_M1 8
#define INT1_M2 15
#define INT2_M2 14
#define INT3_M2 6
#define INT4_M2 9

// LCD configuration
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Create stepper motor objects
AccelStepper stepper1(AccelStepper::FULL4WIRE, INT1_M1, INT2_M1, INT3_M1, INT4_M1);
AccelStepper stepper2(AccelStepper::FULL4WIRE, INT1_M2, INT2_M2, INT3_M2, INT4_M2);

// Variables to track system state and button presses
bool conveyorRunning = false;      // Indicates if the conveyor system is running
bool lastPress = false;            // Tracks the last state of the start button
bool systemLocked = false;         // Indicates if the system is locked (emergency stop)
bool lcdMessageDisplayed = false;  // Tracks if an LCD message has been displayed

void setup() {
  // Configure pins as input or output
  pinMode(PHOTO_SENSOR_PIN, INPUT);
  pinMode(ROCK_SENSOR_PIN, INPUT);
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(EMERGENCY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MOTOR_RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_ALERT_PIN, OUTPUT);

  // Initialize outputs to LOW
  digitalWrite(MOTOR_RELAY_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_ALERT_PIN, LOW);

  // Configure stepper motor parameters
  stepper1.setMaxSpeed(1000);       // Maximum speed for stepper 1
  stepper1.setAcceleration(500);   // Acceleration for stepper 1
  stepper2.setMaxSpeed(1000);       // Maximum speed for stepper 2
  stepper2.setAcceleration(500);   // Acceleration for stepper 2

  // Initialize the LCD
  lcd.begin(16, 2);  // Set LCD size to 16x2
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press Start Button");
  lcd.setCursor(0, 1);
  lcd.print("System Stopped");  // Initial state message
  delay(1000);
}

void loop() {
  // Read button states
  bool startPressed = digitalRead(START_BUTTON_PIN);  // Start button state
  bool stopPressed = digitalRead(STOP_BUTTON_PIN);    // Stop button state
  bool emergencyPressed = digitalRead(EMERGENCY_BUTTON_PIN);  // Emergency button state

  // Start button logic
  if (startPressed == HIGH && !conveyorRunning && lastPress == LOW) {
    conveyorRunning = true;  // Start the conveyor system
    systemLocked = false;    // Unlock the system
  }
  lastPress = startPressed;  // Update the last start button state

  // Stop button logic
  if (stopPressed == HIGH && conveyorRunning) {
    conveyorRunning = false;  // Stop the conveyor system
    systemLocked = false;     // Unlock the system
  }

  // Emergency stop button logic
  if (emergencyPressed == HIGH && conveyorRunning) {
    conveyorRunning = false;  // Stop the conveyor system
    systemLocked = true;      // Lock the system due to emergency
  }

  // Logic for system operation
  if (conveyorRunning && !systemLocked) {
    stepper1.run();            // Run stepper 1
    stepper2.run();            // Run stepper 2
    stepper1.moveTo(1000);     // Set target position for stepper 1
    stepper2.moveTo(1000);     // Set target position for stepper 2
    lcd.clear();
    lcd.print("SYSTEM STARTED");  // Display system started message
  }

  // Logic for stopped system
  if (!conveyorRunning && !systemLocked) {
    stepper1.stop();  // Stop stepper 1
    stepper2.stop();  // Stop stepper 2
    lcd.setCursor(0, 0);
    lcd.print("SYSTEM STOPPED");  // Display system stopped message
    lcd.setCursor(0, 1);
    lcd.print("PRESS START BTN");
  }

  // Logic for emergency stop
  if (!conveyorRunning && systemLocked) {
    stepper1.stop();  // Stop stepper 1
    stepper2.stop();  // Stop stepper 2
    digitalWrite(BUZZER_PIN, HIGH);  // Activate the buzzer
    lcd.setCursor(0, 0);
    lcd.print("EMERGENCY STOP");  // Display emergency stop message
    lcd.setCursor(0, 1);
    lcd.print("PRESS START BTN");
  }
}
