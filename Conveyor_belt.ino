#include <LiquidCrystal.h>

// Define pins for sensors, buttons, and output devices
#define PHOTO_SENSOR_PIN A0
#define ROCK_SENSOR_PIN A1
#define START_BUTTON_PIN 22
#define STOP_BUTTON_PIN 23
#define EMERGENCY_BUTTON_PIN 24
#define MOTOR_RELAY_PIN 25
#define BUZZER_PIN 26
#define LED_ALERT_PIN 27

// First stepper motor
int INT1_M1 = 17;
int INT2_M1 = 16;
int INT3_M1 = 7;
int INT4_M1 = 8;

// Second stepper motor
int INT1_M2 = 15;
int INT2_M2 = 14;
int INT3_M2 = 6;
int INT4_M2 = 9;

// LCD configuration
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

unsigned long noOreDetectedTime = 0;
bool conveyorRunning = false;

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

  // Configure stepper motor pins
  pinMode(INT1_M1, OUTPUT);
  pinMode(INT2_M1, OUTPUT);
  pinMode(INT3_M1, OUTPUT);
  pinMode(INT4_M1, OUTPUT);
  pinMode(INT1_M2, OUTPUT);
  pinMode(INT2_M2, OUTPUT);
  pinMode(INT3_M2, OUTPUT);
  pinMode(INT4_M2, OUTPUT);

  // Initialize outputs
  digitalWrite(MOTOR_RELAY_PIN, LOW); // Start with motor off
  digitalWrite(BUZZER_PIN, LOW);      // Start with buzzer off
  digitalWrite(LED_ALERT_PIN, LOW);  // Start with LED off

  lcd.begin(16, 2); // Initialize LCD
}

void loop() {
  int photoSensorState = digitalRead(PHOTO_SENSOR_PIN);
  int rockSensorState = digitalRead(ROCK_SENSOR_PIN);
  bool startPressed = digitalRead(START_BUTTON_PIN) == LOW;
  bool stopPressed = digitalRead(STOP_BUTTON_PIN) == LOW;
  bool emergencyPressed = digitalRead(EMERGENCY_BUTTON_PIN) == LOW;

  // Start button logic
  if (startPressed) {
    conveyorRunning = true;

    // Both motors move in the same direction
    digitalWrite(INT1_M1, HIGH);
    digitalWrite(INT2_M1, LOW);
    digitalWrite(INT3_M1, LOW);
    digitalWrite(INT4_M1, HIGH);

    digitalWrite(INT1_M2, HIGH);
    digitalWrite(INT2_M2, LOW);
    digitalWrite(INT3_M2, LOW);
    digitalWrite(INT4_M2, HIGH);

    lcd.clear();
    lcd.print("Conveyor Started");
    delay(7000);
  }

  // Stop button logic
  if (stopPressed) {
    conveyorRunning = false;

    // Stop both motors
    digitalWrite(INT1_M1, LOW);
    digitalWrite(INT2_M1, LOW);
    digitalWrite(INT3_M1, LOW);
    digitalWrite(INT4_M1, LOW);

    digitalWrite(INT1_M2, LOW);
    digitalWrite(INT2_M2, LOW);
    digitalWrite(INT3_M2, LOW);
    digitalWrite(INT4_M2, LOW);

    lcd.clear();
    lcd.print("Conveyor Stopped");
    delay(100);
  }

  // Emergency stop logic
  if (emergencyPressed) {
    conveyorRunning = false;

    // Stop both motors and activate buzzer
    digitalWrite(INT1_M1, LOW);
    digitalWrite(INT2_M1, LOW);
    digitalWrite(INT3_M1, LOW);
    digitalWrite(INT4_M1, LOW);

    digitalWrite(INT1_M2, LOW);
    digitalWrite(INT2_M2, LOW);
    digitalWrite(INT3_M2, LOW);
    digitalWrite(INT4_M2, LOW);

    digitalWrite(BUZZER_PIN, HIGH);

    lcd.clear();
    lcd.print("Emergency Stop!");
    delay(100);
  }

  // Ore detection logic
  if (photoSensorState == HIGH) {
    noOreDetectedTime = millis();
  } else if (millis() - noOreDetectedTime > 600000) { // 10 minutes
    conveyorRunning = false;

    digitalWrite(INT1_M1, LOW);
    digitalWrite(INT2_M1, LOW);
    digitalWrite(INT3_M1, LOW);
    digitalWrite(INT4_M1, LOW);

    digitalWrite(INT1_M2, LOW);
    digitalWrite(INT2_M2, LOW);
    digitalWrite(INT3_M2, LOW);
    digitalWrite(INT4_M2, LOW);

    lcd.clear();
    lcd.print("No Ore: Stopping");
    delay(100);
  }

  // Oversized rock detection logic
  if (rockSensorState == HIGH) {
    conveyorRunning = false;

    // Stop both motors
    digitalWrite(INT1_M1, LOW);
    digitalWrite(INT2_M1, LOW);
    digitalWrite(INT3_M1, LOW);
    digitalWrite(INT4_M1, LOW);

    digitalWrite(INT1_M2, LOW);
    digitalWrite(INT2_M2, LOW);
    digitalWrite(INT3_M2, LOW);
    digitalWrite(INT4_M2, LOW);

    digitalWrite(LED_ALERT_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);

    lcd.clear();
    lcd.print("Oversized Rock!");
    delay(1000);

    digitalWrite(LED_ALERT_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
}
