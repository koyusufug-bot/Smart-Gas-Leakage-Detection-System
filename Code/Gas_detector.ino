#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

String phoneNumber = "";

// =====================================================
// TEMPERATURE SENSOR
// =====================================================

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// =====================================================
// LCD
// =====================================================

LiquidCrystal_I2C lcd(0x27, 20, 4);

// =====================================================
// SENSORS
// =====================================================

#define GAS_1 A0
#define GAS_2 A1
#define GAS_3 A2

#define FLAME 3

// =====================================================
// OUTPUTS
// =====================================================

#define BUZZER 12
#define RELAY 13

// =====================================================
// GSM
// =====================================================

#define sim800 Serial

// =====================================================
// KEYPAD
// =====================================================

const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};

byte rowPins[ROWS] = { 5, 6, 7, 8 };
byte colPins[COLS] = { 9, 10, 11 };

Keypad keypad = Keypad(
  makeKeymap(keys),
  rowPins,
  colPins,
  ROWS,
  COLS
);

// =====================================================
// VARIABLES
// =====================================================

unsigned long prevMillis = 0;

int temperatureTreshold = 0;
int gasTreshold = 800;

int gasValue1 = 0;
int gasValue2 = 0;
int gasValue3 = 0;

float tempC = 0.0;

int fireVal = 0;

bool fireSmsSent = false;
bool tempSmsSent = false;
bool gasSmsSent = false;

// =====================================================
// PHONE NUMBER CHANGE
// =====================================================

unsigned long hashPressStart = 0;
bool hashBeingHeld = false;
bool numberChangeTriggered = false;

// =====================================================
// SETUP
// =====================================================

void setup() {

  sim800.begin(9600);

  sensors.begin();

  lcd.init();
  lcd.backlight();

  pinMode(GAS_1, INPUT);
  pinMode(GAS_2, INPUT);
  pinMode(GAS_3, INPUT);
  pinMode(FLAME, INPUT);

  pinMode(BUZZER, OUTPUT);
  pinMode(RELAY, OUTPUT);

  digitalWrite(BUZZER, LOW);
  digitalWrite(RELAY, LOW);

  // ===================================================
  // LOAD SAVED PHONE NUMBER
  // ===================================================

  phoneNumber = loadPhoneNumber();

  // ===================================================
  // LOAD THRESHOLDS
  // ===================================================

  temperatureTreshold = getThreshold("Temperature");
  // gasTreshold = getThreshold("Gas");

  // ===================================================
  // CHECK PHONE NUMBER
  // ===================================================

  if (phoneNumber.length() == 0) {

    // No number saved
    enterPhoneSetup();

  } else {

    // Number already exists
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Saved Number:"));

    lcd.setCursor(0, 1);
    lcd.print(phoneNumber);

    delay(2000);

    lcd.clear();
  }

  // ===================================================
  // STARTUP OPTION TO CHANGE NUMBER
  // ===================================================

  startupNumberChangeCheck();
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  // Check if user wants to change phone number
  checkRunningNumberChange();

  // ===================================================
  // READ SENSORS EVERY 3 SECONDS
  // ===================================================

  if (millis() - prevMillis > 3000) {

    sensors.requestTemperatures();

    tempC = sensors.getTempCByIndex(0);

    gasValue1 = analogRead(GAS_1);
    gasValue2 = analogRead(GAS_2);
    gasValue3 = analogRead(GAS_3);

    fireVal = digitalRead(FLAME);

    prevMillis = millis();
  }

  // ===================================================
  // DETECTION
  // ===================================================

  bool gasDetected =
    /* gasValue1 > gasTreshold || */
    gasValue2 > gasTreshold ||
    gasValue3 > gasTreshold;

  bool highTemp = tempC > temperatureTreshold && fireVal == LOW;

  // ===================================================
  // LCD DISPLAY
  // ===================================================

  lcd.setCursor(0, 0);
  lcd.print(F("GAS "));
  lcd.print(gasDetected ? "Detected     " : "Not Detected ");

  lcd.setCursor(0, 1);
  lcd.print(F("FLAME "));
  lcd.print(
    highTemp ? "Detected     " : "Not Detected "
  );

  lcd.setCursor(0, 2);
  lcd.print(F("Temp:"));
  lcd.print(tempC, 1);
  lcd.print(F(" C       "));

  // lcd.setCursor(0, 3);
  // lcd.print(F("Temp "));
  // lcd.print(highTemp ? "HIGH   " : "Normal     ");

  // ===================================================
  // GAS ALERT
  // ===================================================

  if (gasDetected) {

    if (!gasSmsSent) {

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print(F("Gas Detected"));

      lcd.setCursor(0, 1);
      lcd.print(F("Sending SMS...."));

      gasSmsSent = true;

      sendSMS("Alert!!!\nGas Detected!!!");

      delay(2000);

      lcd.clear();
    }

    playAlert();

  } else {

    gasSmsSent = false;
  }

  // ===================================================
  // FIRE ALERT
  // ===================================================

  if (highTemp) {

    digitalWrite(RELAY, HIGH);

    if (!fireSmsSent) {

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print(F("Fire Detected"));

      lcd.setCursor(0, 1);
      lcd.print(F("Sending SMS...."));

      sendSMS("Alert!!!\nFire Detected!!!");

      fireSmsSent = true;

      delay(2000);

      lcd.clear();
    }

    playAlert();

  } else {

    digitalWrite(RELAY, LOW);

    fireSmsSent = false;
  }

  // ===================================================
  // HIGH TEMPERATURE ALERT
  // ===================================================

  // if (highTemp) {

  //   if (!tempSmsSent) {

  //     lcd.clear();

  //     lcd.setCursor(0, 0);
  //     lcd.print(F("High Temp Detected"));

  //     lcd.setCursor(0, 1);
  //     lcd.print(F("Sending SMS...."));

  //     tempSmsSent = true;

  //     sendSMS("Alert!!!\nHigh Temperature Detected!!!");

  //     delay(2000);

  //     lcd.clear();
  //   }

  //   playAlert();

  // } else {

  //   tempSmsSent = false;
  // }
}

// =====================================================
// KEYPAD THRESHOLD INPUT
// =====================================================

int getThreshold(String label) {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(label);

  lcd.setCursor(0, 1);
  lcd.print(F("Enter value: "));

  String input = "";

  while (true) {

    char key = keypad.getKey();

    if (key != NO_KEY) {

      // Save
      if (key == '#') {

        int value = input.toInt();

        lcd.clear();

        lcd.print(F("Saved: "));
        lcd.print(value);

        delay(1000);

        return value;
      }

      // Clear
      else if (key == '*') {

        input = "";

        lcd.setCursor(0, 1);
        lcd.print(F("Cleared         "));
      }

      // Number
      else if (isDigit(key)) {

        input += key;

        lcd.setCursor(0, 1);
        lcd.print(input);
        lcd.print(F("              "));
      }
    }
  }
}

// =====================================================
// SEND SMS
// =====================================================

void sendSMS(String message) {

  // Make sure a number exists
  if (phoneNumber.length() == 0) {
    return;
  }

  sim800.println(F("AT+CMGF=1"));

  delay(500);

  sim800.print(F("AT+CMGS=\""));
  sim800.print(phoneNumber);
  sim800.println(F("\""));

  delay(500);

  sim800.print(message);

  sim800.write(26);

  delay(1000);
}

// =====================================================
// BUZZER
// =====================================================

void playAlert() {

  digitalWrite(BUZZER, HIGH);

  delay(4000);

  digitalWrite(BUZZER, LOW);
}

// =====================================================
// SAVE PHONE NUMBER
// =====================================================

void savePhoneNumber(String number) {

  // Maximum 15 characters
  if (number.length() > 15) {
    number = number.substring(0, 15);
  }

  // Store length at address 0
  EEPROM.update(0, number.length());

  // Store phone number
  for (int i = 0; i < number.length(); i++) {
    EEPROM.update(i + 1, number[i]);
  }

  // Clear any old characters
  for (int i = number.length() + 1; i <= 16; i++) {
    EEPROM.update(i, 0);
  }
}

// =====================================================
// LOAD PHONE NUMBER
// =====================================================

String loadPhoneNumber() {

  String number = "";

  int len = EEPROM.read(0);

  // Validate stored length
  if (len <= 0 || len > 15) {
    return "";
  }

  for (int i = 0; i < len; i++) {

    char c = EEPROM.read(i + 1);

    // Basic validation
    if (c == 0 || c == 255) {
      return "";
    }

    number += c;
  }

  return number;
}

// =====================================================
// PHONE NUMBER SETUP
// =====================================================

void enterPhoneSetup() {

  String number = "";

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(F("Enter Number"));

  lcd.setCursor(0, 1);
  lcd.print(F("                "));

  delay(500);

  while (true) {

    char key = keypad.getKey();

    if (key != NO_KEY) {

      // =================================================
      // DIGIT
      // =================================================

      if (key >= '0' && key <= '9') {

        if (number.length() < 15) {
          number += key;
        }
      }

      // =================================================
      // DELETE LAST DIGIT
      // =================================================

      else if (key == '*') {

        if (number.length() > 0) {
          number.remove(number.length() - 1);
        }
      }

      // =================================================
      // SAVE
      // =================================================

      else if (key == '#') {

        if (number.length() >= 10) {

          // Add + before saving
          phoneNumber = "+" + number;

          // Save the SAME value that is used by SMS
          savePhoneNumber(phoneNumber);

          lcd.clear();

          lcd.setCursor(0, 0);
          lcd.print(F("Number Saved"));

          lcd.setCursor(0, 1);
          lcd.print(phoneNumber);

          delay(2000);

          lcd.clear();

          return;
        }

        else {

          lcd.clear();

          lcd.setCursor(0, 0);
          lcd.print(F("Invalid Number"));

          lcd.setCursor(0, 1);
          lcd.print(F("Min 10 digits"));

          delay(1500);
        }
      }

      // =================================================
      // DISPLAY
      // =================================================

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print(F("Phone Number"));

      lcd.setCursor(0, 1);
      lcd.print(number);
    }
  }
}

// =====================================================
// STARTUP NUMBER CHANGE
// =====================================================

void startupNumberChangeCheck() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(F("Hold # to change"));

  lcd.setCursor(0, 1);
  lcd.print(F("Phone Number"));

  unsigned long startTime = millis();

  bool changed = false;

  while (millis() - startTime < 3000) {

    char key = keypad.getKey();

    if (key == '#') {

      unsigned long pressStart = millis();

      // Wait while # is being pressed
      while (millis() - pressStart < 2000) {

        char k = keypad.getKey();

        if (k != NO_KEY && k != '#') {
          break;
        }
      }

      if (millis() - pressStart >= 2000) {

        enterPhoneSetup();

        changed = true;
      }

      break;
    }
  }

  if (!changed) {
    lcd.clear();
  }
}

// =====================================================
// CHANGE NUMBER WHILE SYSTEM IS RUNNING
// =====================================================

void checkRunningNumberChange() {

  char key = keypad.getKey();

  if (key == '#') {

    unsigned long pressStart = millis();

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print(F("Hold #"));

    lcd.setCursor(0, 1);
    lcd.print(F("Change Number"));

    // Wait for 2 seconds
    while (millis() - pressStart < 2000) {

      char currentKey = keypad.getKey();

      // If another key is pressed, cancel
      if (currentKey != NO_KEY && currentKey != '#') {

        lcd.clear();

        return;
      }
    }

    // Long press detected
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print(F("Changing Number"));

    delay(500);

    enterPhoneSetup();

    // Prevent immediate re-trigger
    delay(500);
  }
}
