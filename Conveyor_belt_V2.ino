#include <AccelStepper.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
// Pin definitions
#define START_BUTTON_PIN 22             // Start Button
#define STOP_BUTTON_PIN 23              // Stop Button
#define EMERGENCY_STOP_PIN 26           // Emergency Stop (Normally Closed)
#define BUZZER_PIN 13                   // Buzzer
#define PHOTOELECTRIC_SENSOR_PIN_1 25   // Ore Detection Sensor (Sensor 1)
#define PHOTOELECTRIC_SENSOR_PIN_2 24   // Large Ore Detection Sensor (Sensor 2)
#define RELAY_START_PIN 10              // Relay for Start Light (IN_1)
#define RELAY_STOP_PIN 11               // Relay for Stop Light (IN_2)

#define INT1_CONVEYOR_STEPPER 17    // Conveyor Stepper, Pulse/Step (PUL+ for DM542)
#define INT2_CONVEYOR_STEPPER 16    // Conveyor Stepper, Direction (DIR+ for DM542)
#define INT1_LARGE_ORE_STEPPER 15   // Large Ore Stepper, Pulse/Step (PUL+ for DM542)
#define INT2_LARGE_ORE_STEPPER 14   // Large Ore Stepper, Direction (DIR+ for DM542)

AccelStepper conveyorStepper(AccelStepper::DRIVER, INT1_CONVEYOR_STEPPER, INT2_CONVEYOR_STEPPER);
AccelStepper largeOreStepper(AccelStepper::DRIVER, INT1_LARGE_ORE_STEPPER, INT2_LARGE_ORE_STEPPER);

bool systemRunning = false;        // Tracks whether the system is running
bool conveyorRunning = false;      // Tracks Conveyor Stepper motor status
bool lastStartPress = false;       // Tracks the last state of the Start Button
bool lastStopPress = false;        // Tracks the last state of the Stop Button
bool lastSensor1State = true;      // Tracks the last state of Sensor 1 (Ore Detection Sensor)
bool lastSensor2State = true;      // Tracks the last state of Sensor 2 (Large Ore Detection Sensor)

unsigned long sensor1OpenedTime = 0;       // Timer for when Sensor 1 opens
unsigned long sensor2OpenedTime = 0;       // Timer for when Sensor 2 opens
unsigned long motorStopDelayTime = 60000;  // Delay time of 1 minute (60,000 ms)

void setup() {
  // Configure pins
  Serial.begin(9600);
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(EMERGENCY_STOP_PIN, INPUT_PULLUP); // Normally Closed Switch
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_START_PIN, OUTPUT);
  pinMode(RELAY_STOP_PIN, OUTPUT);
  pinMode(PHOTOELECTRIC_SENSOR_PIN_1, INPUT_PULLUP);  // Ore Detection Sensor
  pinMode(PHOTOELECTRIC_SENSOR_PIN_2, INPUT_PULLUP);  // Large Ore Detection Sensor

  // Initialize relays
  digitalWrite(RELAY_START_PIN, HIGH); // Ensure the Start light is off
  digitalWrite(RELAY_STOP_PIN, HIGH); // Ensure the Stop light is on

  // Configure stepper motors
  conveyorStepper.setMaxSpeed(1000);
  conveyorStepper.setAcceleration(500);
  largeOreStepper.setMaxSpeed(1000);
  largeOreStepper.setAcceleration(500);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Initializing");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
}

void buzz(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}

void loop() {
  // Read button and sensor states
  bool startPressed = digitalRead(START_BUTTON_PIN) == LOW;
  bool stopPressed = digitalRead(STOP_BUTTON_PIN) == LOW;
  bool emergencyStopOpened = digitalRead(EMERGENCY_STOP_PIN) ; // HIGH = open (system shutdown)
  bool oreSensorClosed = digitalRead(PHOTOELECTRIC_SENSOR_PIN_1);  // Sensor 1
  bool largeOreSensorClosed = digitalRead(PHOTOELECTRIC_SENSOR_PIN_2) ;  // Sensor 2

  // Emergency Stop logic: shut down the system if the switch is open
  if (emergencyStopOpened == HIGH) {
    buzz(500);                // Buzz for emergency stop
    systemRunning = false;    // Stop the entire system
    conveyorRunning = false;  // Stop the conveyor
    conveyorStepper.stop();   // Stop Conveyor Stepper motor
    largeOreStepper.stop();   // Stop Large Ore Stepper motor
    Serial.print("Emergency stop");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Emergency stop");

    // Update relays
    digitalWrite(RELAY_START_PIN, HIGH); // Turn off Start light
    digitalWrite(RELAY_STOP_PIN, LOW); // Turn on Stop light
    return;                   // Exit the loop immediately to ensure no further actions
  }

  // Stop Button logic: stop the system immediately
  if (stopPressed && !lastStopPress) {
    buzz(500);                // Buzz before stopping motors
    systemRunning = false;    // Stop the entire system
    conveyorRunning = false;  // Stop the conveyor
    conveyorStepper.stop();   // Stop Conveyor Stepper motor
    largeOreStepper.stop();   // Stop Large Ore Stepper motor
     lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Stop Button Pressed");
      lcd.setCursor(5, 1);
    lcd.print("System top");

    // Update relays
    digitalWrite(RELAY_START_PIN, HIGH); // Turn off Start light
    digitalWrite(RELAY_STOP_PIN, LOW); // Turn on Stop light
  }
  lastStopPress = stopPressed;

  // Start Button logic: system starts only when the button is pressed and Sensor 1 is closed
  if (startPressed && !lastStartPress) {
    buzz(500);                // Buzz before starting motors
    systemRunning = true;     // Start the system
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Start Button Pressed");
    lcd.setCursor(4, 1);
    lcd.print("System Started");

    // Update relays
    digitalWrite(RELAY_START_PIN, LOW); // Turn on Start light
    digitalWrite(RELAY_STOP_PIN, HIGH);  // Turn off Stop light
  }
  lastStartPress = startPressed;

  // System operation logic
  if (systemRunning =true)  {
    // Conveyor motor logic
    if (oreSensorClosed== LOW) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Started");
    lcd.setCursor(0, 1);
    lcd.print("Conveyor Running");
    lcd.setCursor(0, 2);
    lcd.print("Ore Detected ");
      conveyorRunning = true;
      conveyorStepper.setSpeed(400);
      conveyorStepper.runSpeed();
    } else if(oreSensorClosed== HIGH)  {//No ore detected
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Started");
    lcd.setCursor(0, 1);
    lcd.print("Conveyor Not Running");
    lcd.setCursor(0, 2);
    lcd.print("No Ore Detected ");
        conveyorRunning = false;  // Stop the conveyor IF NO ore is detected
        conveyorStepper.stop();  
    }
    // Large Ore Stepper motor logic
    if (largeOreSensorClosed== LOW) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Started");
    lcd.setCursor(0, 1);
    lcd.print("Conveyor  Running");
    lcd.setCursor(0, 2);
    lcd.print(" Large Ore Detected ");
      sensor2OpenedTime = 0;  // Reset the timer when Sensor 2 is closed
      largeOreStepper.setSpeed(400);
      largeOreStepper.runSpeed();
    } 
      // If Sensor 2 is open, start the delay timer
      if (sensor2OpenedTime == 0) {
        sensor2OpenedTime = millis();  // Start the 1-minute timer
      } else if (millis() - sensor2OpenedTime >= motorStopDelayTime) {
        largeOreStepper.stop();  // Stop Large Ore Stepper motor after 1 minute
      }
   }
   else {
    // Stop all motors when the system is not running
    conveyorStepper.stop();
    largeOreStepper.stop();
  }
  
}