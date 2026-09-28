/*
 * Smart Biometric & IoT Door Lock System
 * --------------------------------------
 * Core functionality:
 * - Matrix Keypad (4x4)
 * - Optical Fingerprint Sensor (JM-101 / R307)
 * - 16x2 / 20x4 I2C LCD
 * - Bluetooth HC-06 (Software Serial or Hardware Serial)
 * - Solenoid Door Lock (via 5V Relay)
 * 
 * Auto-generated Boilerplate.
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// ==========================================
// PIN DEFINITIONS
// ==========================================
#define RELAY_PIN 12

// Fingerprint Sensor Pins (SoftwareSerial)
#define FINGER_RX 2
#define FINGER_TX 3

// ==========================================
// MODULE INITIALIZATIONS
// ==========================================

// 1. LCD Display (Address 0x27 is common, change to 0x3F if it doesn't work)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// 2. Fingerprint Sensor
SoftwareSerial mySerial(FINGER_RX, FINGER_TX);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// 3. Matrix Keypad Setup
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {11, 10, 9, 8}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 6, 5, 4};   // Connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ==========================================
// SYSTEM STATE
// ==========================================
String inputPassword = "";
String correctPassword = "1234"; // Default password

void setup() {
  // Initialize Serial Monitor for Debugging
  Serial.begin(9600);
  while (!Serial);
  Serial.println("\n--- Smart Door Lock System ---");

  // Initialize Relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Ensure lock is SECURED on startup (assuming HIGH = unlock)

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Booting...");

  // Initialize Fingerprint Sensor
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
    lcd.setCursor(0, 1);
    lcd.print("FP Sensor: OK");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    lcd.setCursor(0, 1);
    lcd.print("FP Sensor: ERROR");
  }
  
  delay(2000);
  lcd.clear();
  displayWelcomeMessage();
}

void loop() {
  // 1. Check for Keypad Input
  char key = keypad.getKey();
  if (key) {
    handleKeypadInput(key);
  }

  // 2. Check for Fingerprint Match
  // Uncomment when ready to implement continuous scanning
  // checkFingerprint();
  
  // 3. Check for Bluetooth Commands (via Hardware Serial RX/TX)
  if (Serial.available()) {
    char btCommand = Serial.read();
    if (btCommand == 'U') {
      unlockDoor();
    }
  }
}

// ==========================================
// HELPER FUNCTIONS
// ==========================================

void displayWelcomeMessage() {
  lcd.setCursor(0, 0);
  lcd.print("Enter PIN or");
  lcd.setCursor(0, 1);
  lcd.print("Scan Finger");
}

void handleKeypadInput(char key) {
  if (key == '#') {
    // Submit Password
    if (inputPassword == correctPassword) {
      lcd.clear();
      lcd.print("Access Granted!");
      unlockDoor();
    } else {
      lcd.clear();
      lcd.print("Access Denied!");
      delay(2000);
      lcd.clear();
      displayWelcomeMessage();
    }
    inputPassword = ""; // Reset
  } 
  else if (key == '*') {
    // Clear Input
    inputPassword = "";
    lcd.clear();
    displayWelcomeMessage();
  } 
  else {
    // Append to password
    inputPassword += key;
    lcd.clear();
    lcd.print("PIN: ");
    for(int i=0; i<inputPassword.length(); i++){
      lcd.print("*");
    }
  }
}

void unlockDoor() {
  Serial.println("Door Unlocked!");
  digitalWrite(RELAY_PIN, HIGH); // Trigger Relay
  delay(5000);                   // Keep open for 5 seconds
  digitalWrite(RELAY_PIN, LOW);  // Lock again
  Serial.println("Door Locked.");
  
  lcd.clear();
  displayWelcomeMessage();
}
