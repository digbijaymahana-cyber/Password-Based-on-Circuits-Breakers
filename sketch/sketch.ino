/*
  Password Based Circuit Breaker (model)
  Board   : Arduino Uno
  Parts   : 4x4 keypad, 16x2 I2C LCD, relay module, buzzer, 2 LEDs
  Libs    : Keypad, LiquidCrystal_I2C

  Usage:
    - Type the password, then press '#'  -> breaker toggles ON/OFF
    - '*' clears what you typed
    - 3 wrong attempts -> keypad locked for 30 s with buzzer
*/

#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------- Pins ----------
const byte RELAY_PIN  = 10;   // relay IN (active LOW for most modules)
const byte BUZZER_PIN = 11;
const byte GREEN_LED  = 12;   // breaker ON
const byte RED_LED    = 13;   // breaker OFF

// ---------- Keypad ----------
const byte ROWS = 4, COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ---------- LCD (change 0x27 to 0x3F if blank) ----------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------- Settings ----------
const String PASSWORD = "1234";        // change this
const byte   MAX_TRIES = 3;
const unsigned long LOCK_TIME = 30000; // ms

String input = "";
bool breakerOn = false;
byte wrongCount = 0;

void setBreaker(bool on) {
  breakerOn = on;
  digitalWrite(RELAY_PIN, on ? LOW : HIGH);   // flip if your relay is active HIGH
  digitalWrite(GREEN_LED, on);
  digitalWrite(RED_LED, !on);
}

void showHome() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Breaker: ");
  lcd.print(breakerOn ? "ON " : "OFF");
  lcd.setCursor(0, 1);
  lcd.print("Pass: ");
}

void beep(int ms) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(ms);
  digitalWrite(BUZZER_PIN, LOW);
}

void lockout() {
  lcd.clear();
  lcd.print("LOCKED!");
  unsigned long start = millis();
  while (millis() - start < LOCK_TIME) {
    lcd.setCursor(0, 1);
    lcd.print("Wait ");
    lcd.print((LOCK_TIME - (millis() - start)) / 1000);
    lcd.print("s   ");
    beep(100);
    delay(400);
  }
  wrongCount = 0;
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  lcd.init();
  lcd.backlight();

  setBreaker(false);          // start with breaker OFF
  showHome();
}

void loop() {
  char key = keypad.getKey();
  if (!key) return;

  if (key == '#') {                       // submit
    if (input == PASSWORD) {
      wrongCount = 0;
      setBreaker(!breakerOn);
      lcd.clear();
      lcd.print("Access Granted");
      lcd.setCursor(0, 1);
      lcd.print(breakerOn ? "Breaker CLOSED" : "Breaker TRIPPED");
      beep(200);
    } else {
      wrongCount++;
      lcd.clear();
      lcd.print("Wrong Password");
      lcd.setCursor(0, 1);
      lcd.print("Tries left: ");
      lcd.print(MAX_TRIES - wrongCount);
      beep(600);
      delay(1500);
      if (wrongCount >= MAX_TRIES) lockout();
    }
    input = "";
    delay(1500);
    showHome();
  }
  else if (key == '*') {                  // clear
    input = "";
    showHome();
  }
  else if (input.length() < 8) {          // add digit
    input += key;
    lcd.setCursor(6 + input.length() - 1, 1);
    lcd.print('*');                       // mask password
  }
}