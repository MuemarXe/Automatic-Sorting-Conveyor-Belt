#include<arduino.h>
#include <LiquidCrystal.h>
#include <AccelStepper.h>

// Define pins for sensors, buttons, and output devices
#define PHOTO_SENSOR_PIN A0
#define ROCK_SENSOR_PIN A1
#define START_BUTTON_PIN 22
#define STOP_BUTTON_PIN 23
#define EMERGENCY_BUTTON_PIN 24
#define MOTOR_RELAY_PIN 25
#define BUZZER_PIN 26
#define LED_ALERT_PIN 27

// Stepper motor pin definitions
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

// Create stepper objects
AccelStepper stepper1(AccelStepper::FULL4WIRE, INT1_M1, INT2_M1, INT3_M1, INT4_M1);
AccelStepper stepper2(AccelStepper::FULL4WIRE, INT1_M2, INT2_M2, INT3_M2, INT4_M2);

// Variables
bool conveyorRunning = false; // System state

void setup() {
  // Configure pins
  pinMode(PHOTO_SENSOR_PIN, INPUT);
  pinMode(ROCK_SENSOR_PIN, INPUT);
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(EMERGENCY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MOTOR_RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_ALERT_PIN, OUTPUT);

  // Initialize outputs
  digitalWrite(MOTOR_RELAY_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_ALERT_PIN, LOW);

  // Configure stepper motors
  stepper1.setMaxSpeed(1000);
  stepper1.setAcceleration(500);
  stepper2.setMaxSpeed(1000);
  stepper2.setAcceleration(500);

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Press Start Button");
  lcd.setCursor(0,1);
  lcd.print("System Stopped"); // Initial state
}

void loop() {
  // Read button states
  bool startPressed = digitalRead(START_BUTTON_PIN) ;
  bool stopPressed = digitalRead(STOP_BUTTON_PIN) ;
  bool emergencyPressed = digitalRead(EMERGENCY_BUTTON_PIN);

  // Start button logic
  if (startPressed ==HIGH && !conveyorRunning) {
    conveyorRunning = true; // Change system state

    // Move both motors forward
    stepper1.moveTo(1000);
    stepper2.moveTo(1000);

    lcd.clear();
    lcd.print("System Started");
  }

  // Run motors if conveyor is running
  if (conveyorRunning) {
    stepper1.run();
    stepper2.run();
  }

  // Stop button logic
  if (stopPressed ==LOW && conveyorRunning) {
    conveyorRunning = false; // Change system state

    // Stop both motors
    stepper1.stop();
    stepper2.stop();

    lcd.clear();
    lcd.print("System Stopped");
  }

  // Emergency stop logic
  if (emergencyPressed ==LOW && conveyorRunning) {
    conveyorRunning = false; // Change system state

    // Stop both motors and activate buzzer
    stepper1.stop();
    stepper2.stop();

    digitalWrite(BUZZER_PIN, HIGH);

    lcd.clear();
    lcd.print("Emergency Stop!");
    delay(1000);

    digitalWrite(BUZZER_PIN, LOW);

    lcd.clear();
    lcd.print("System Stopped");
  }

  // Logic for restarting after stopping
  if (!conveyorRunning && startPressed) {
    conveyorRunning = true;

    // Restart both motors
    stepper1.moveTo(1000);
    stepper2.moveTo(1000);

    lcd.clear();
    lcd.print("System Restarted");
  }
}
