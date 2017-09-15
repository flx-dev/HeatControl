/**********************************************
* Date: 2017-09-13
* File: HeatControl.ino
* Description: Arduino based heat mixture
*   control. Watches every X (e.g. 30) seconds
*   if temperature out of range or going in
*   wrong direction and adjust the mixture
***********************************************/

#include<LiquidCrystal.h>
#include<LCDKeypad.h>
#include<OneWire.h>
#include<DallasTemperature.h>

// PIN and CONSTANT Definitions
#define OUTPUT0 11               // Open
#define OUTPUT1 12               // Close
#define TEMPPIN 2                // DS18B20

#define CONTROL_ACTIVE_TIME 1000 // [ms]
#define CONTROL_LOOP_TIME 30000  // [ms]
#define BUTTON_DEAD_TIME 500     // [ms]

// Global Variable Declarations
unsigned long timer[4] = {0, 0, 0, 0};
bool outputState[4] = {false, false, true, true}; // +, -, + old, - old
float T[5] = {26, 26, 0, 0, 0}; // Temperature {current, target, old, lcdCurrent, lcdTarget}
float dT = 0, dTold = 1;

OneWire oneWire(TEMPPIN);
DallasTemperature sensors(&oneWire);
LCDKeypad lcd;

byte c_up[8] = { B00100, B01110, B10101, B00100, B00100, B00100, B00100, B00100 };
byte c_down[8] = { B00100, B00100, B00100, B00100, B00100, B10101, B01110, B00100 };


void setup() {
  // Define Pinmodes
  pinMode(OUTPUT0, OUTPUT);
  pinMode(OUTPUT1, OUTPUT);
  digitalWrite(OUTPUT0, LOW);
  digitalWrite(OUTPUT1, LOW);

  // Define LCD Button
  lcd.createChar(1,c_up);
  lcd.createChar(2,c_down);
  lcd.begin(16, 2);
  lcd.clear();
  
  lcd.setCursor(0,0);
  lcd.print("S:");
  lcd.setCursor(0,1);
  lcd.print("I:");
  lcd.setCursor(7,0);
  lcd.print("dT:");
  lcd.setCursor(11,1);
  lcd.print("+");
  lcd.setCursor(14,1);
  lcd.print("-");
  printLcd();
  sensors.begin();
}

void loop() {
  readTemperature();
  readButton();
  checkState();
  close();
  printLcd();
}


void printLcd() {
  if(T[4] - T[1] != 0) {
    lcd.setCursor(3,0);
    lcd.print((int)T[1]);
    T[4] = T[1];
  }

  if(T[3] - T[0] != 0) {
    lcd.setCursor(3,1);
    lcd.print(T[0]);
    T[3] = T[0];
  }

  if(outputState[0] != outputState[2]) {
    lcd.setCursor(12,1);
    lcd.print(outputState[0]);
    outputState[2] = outputState[0];
  }

  if(outputState[1] != outputState[3]) {
    lcd.setCursor(15,1);
    lcd.print(outputState[1]);
    outputState[3] = outputState[1];
  }

  if(dT != dTold) {
    lcd.setCursor(11,0);
    lcd.print(dT);
    dTold = dT;
  }
}

void close() {
  if(outputState[0] == true && millis() - timer[0] > CONTROL_ACTIVE_TIME) {
    digitalWrite(OUTPUT0, LOW);
    outputState[0] = false;
  } else if (outputState[1] == true && millis() - timer[0] > CONTROL_ACTIVE_TIME) {
    digitalWrite(OUTPUT1, LOW);
    outputState[1] = false;
  }
}

void checkState() {
  if(millis() - timer[1] > CONTROL_LOOP_TIME) {
    if(T[1] - T[0] > 2.0 || T[0] < T[2]) {
      // Open for CONTROL_ACTIVE_TIME
      if(T[0] > 0 && outputState[0] == false && outputState[1] == false) {
        digitalWrite(OUTPUT0, HIGH);
        outputState[0] = true;
        timer[0] = millis();
      }
    } else if (T[1] - T[0] < -2.0 || T[0] > T[2]) {
      // Close for CONTROL_ACTIVE_TIME
      if(T[0] > 0 && outputState[0] == false && outputState[1] == false) {
        digitalWrite(OUTPUT1, HIGH);
        outputState[1] = true;
        timer[0] = millis();
      }
    }
    timer[1] = millis();
    dT = (T[0] - T[2]) / CONTROL_LOOP_TIME * 60.0 * 1000.0;
    T[2] = T[0];
  }
  if (T[0] <= 0) {
    digitalWrite(OUTPUT0, LOW);
    digitalWrite(OUTPUT1, LOW);
    outputState[0] = false;
    outputState[1] = false;
  }
}

void readButton() {
  // If timer, read button (button deadtime)
  // if pressed, increase temperature by one in range [20,37]
  if(millis() - timer[2] > BUTTON_DEAD_TIME) {
    int button = lcd.button();
    if (button == 1) {
      T[1] = ((int)T[1] - 20 + 1)%18 + 20;
      timer[2] = millis();
    } else if (button == 2) {
      T[1]--;
      if(T[1] == 19) T[1] = 37;
      timer[2] = millis();
    }
  }
}

void readTemperature() {
  if(millis() - timer[3] > 1000) {
    sensors.requestTemperatures();
    T[0] = sensors.getTempCByIndex(0);
    timer[3] = millis();
  }
}