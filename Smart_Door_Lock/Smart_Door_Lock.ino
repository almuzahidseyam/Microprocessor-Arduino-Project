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
 * Non-blocking Architecture using millis() for multitasking.
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
#define FINGER_RX 2
#define FINGER_TX 3

// ==========================================
// MODULE INITIALIZATIONS
// ==========================================
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial mySerial(FINGER_RX, FINGER_TX);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {11, 10, 9, 8};
byte colPins[COLS] = {7, 6, 5, 4};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ==========================================
// SYSTEM STATE (NON-BLOCKING)
// ==========================================
String inputPassword = "";
String correctPassword = "1234";

// State Machine Variables
bool isDoorUnlocked = false;
unsigned long doorUnlockStartTime = 0;
const unsigned long DOOR_UNLOCK_DURATION = 5000; // 5 seconds

bool isDisplayingError = false;
unsigned long errorDisplayStartTime = 0;
const unsigned long ERROR_DISPLAY_DURATION = 2000; // 2 seconds

bool lcdNeedsUpdate = false;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("\n--- Smart Door Lock System (Non-Blocking) ---");

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Locked

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Booting...");

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
  
  delay(2000); // Only blocking delay allowed is during setup boot
  lcd.clear();
  displayWelcomeMessage();
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. NON-BLOCKING LOCK TIMEOUT
  if (isDoorUnlocked) {
    if (currentMillis - doorUnlockStartTime >= DOOR_UNLOCK_DURATION) {
      digitalWrite(RELAY_PIN, LOW); // Lock again
      isDoorUnlocked = false;
      Serial.println("Door Locked (Timeout).");
      lcdNeedsUpdate = true;
    }
  }

  // 2. NON-BLOCKING ERROR MESSAGE TIMEOUT
  if (isDisplayingError) {
    if (currentMillis - errorDisplayStartTime >= ERROR_DISPLAY_DURATION) {
      isDisplayingError = false;
      lcdNeedsUpdate = true;
    }
  }

  // 3. REFRESH LCD IF NEEDED
  if (lcdNeedsUpdate) {
    lcd.clear();
    displayWelcomeMessage();
    lcdNeedsUpdate = false;
  }

  // 4. PROCESS KEYPAD (Only if not displaying a temporary error)
  if (!isDisplayingError) {
    char key = keypad.getKey();
    if (key) {
      handleKeypadInput(key);
    }
  }

  // 5. PROCESS BLUETOOTH
  if (Serial.available()) {
    char btCommand = Serial.read();
    if (btCommand == 'U') {
      triggerUnlock();
    }
  }
}

// ==========================================
// HELPER FUNCTIONS
// ==========================================

void displayWelcomeMessage() {
  if (isDoorUnlocked) {
    lcd.setCursor(0, 0);
    lcd.print("Door Unlocked!");
    lcd.setCursor(0, 1);
    lcd.print("Come inside...");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Enter PIN or");
    lcd.setCursor(0, 1);
    lcd.print("Scan Finger");
  }
}

void handleKeypadInput(char key) {
  if (key == '#') {
    if (inputPassword == correctPassword) {
      triggerUnlock();
    } else {
      lcd.clear();
      lcd.print("Access Denied!");
      isDisplayingError = true;
      errorDisplayStartTime = millis();
    }
    inputPassword = "";
  } 
  else if (key == '*') {
    inputPassword = "";
    lcdNeedsUpdate = true;
  } 
  else {
    inputPassword += key;
    lcd.clear();
    lcd.print("PIN: ");
    for(int i = 0; i < inputPassword.length(); i++){
      lcd.print("*");
    }
  }
}

void triggerUnlock() {
  Serial.println("Door Unlocked!");
  digitalWrite(RELAY_PIN, HIGH);
  isDoorUnlocked = true;
  doorUnlockStartTime = millis();
  
  lcd.clear();
  displayWelcomeMessage();
}
