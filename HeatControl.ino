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
#define output0 11               // Open
#define output1 12               // Close
#define tempPin 2                // DS18B20

#define CONTROL_ACTIVE_TIME 1000 // [ms]
#define CONTROL_LOOP_TIME 30000  // [ms]
#define BUTTON_DEAD_TIME 500     // [ms]

// Global Variable Declarations
unsigned long timer_temperature = 0;
unsigned long timer_button = 0;
unsigned long timer_state = 0;
bool outputActive[2] = {false, false};
bool lcdOutputActive[2] = {true, true};
unsigned long activated = 0;
float temperature = 26;                 // Current temperature
float temperatureOld = 25;              // Temperature before CONTROL_LOOP_TIME
float lcdTempTarget = 0;                 // Current temperature
float lcdTempOrigin = 0;              // Temperature before CONTROL_LOOP_TIME
int targetTemperature = 26;             // Target temperature
int lcdtime = 0;
float dT = 0, dTold = 1;

OneWire oneWire(tempPin);
DallasTemperature sensors(&oneWire);
LCDKeypad lcd;

byte c_up[8] = { B00100, B01110, B10101, B00100, B00100, B00100, B00100, B00100 };
byte c_down[8] = { B00100, B00100, B00100, B00100, B00100, B10101, B01110, B00100 };


void setup() {
  // Define Pinmodes
  pinMode(output0, OUTPUT);
  pinMode(output1, OUTPUT);
  digitalWrite(output0, LOW);
  digitalWrite(output1, LOW);

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
  readButton();
  checkState();
  close();
  printLcd();
}


void printLcd() {
  if(lcdTempTarget - targetTemperature != 0) {
    lcd.setCursor(3,0);
    lcd.print((int)targetTemperature);
    lcdTempTarget = targetTemperature;
  }

  if(lcdTempOrigin - temperature != 0) {
    lcd.setCursor(3,1);
    lcd.print(temperature);
    lcdTempOrigin = temperature;
  }

  if(outputActive[0] != lcdOutputActive[0]) {
    lcd.setCursor(12,1);
    lcd.print(outputActive[0]);
    lcdOutputActive[0] = outputActive[0];
  }

  if(outputActive[1] != lcdOutputActive[1]) {
    lcd.setCursor(15,1);
    lcd.print(outputActive[1]);
    lcdOutputActive[1] = outputActive[1];
  }

  if(dT != dTold) {
    lcd.setCursor(11,0);
    lcd.print(dT);
    dTold = dT;
  }
}

void close() {
  if(outputActive[0] == true && millis() - activated > CONTROL_ACTIVE_TIME) {
    digitalWrite(output0, LOW);
    outputActive[0] = false;
  } else if (outputActive[1] == true && millis() - activated > CONTROL_ACTIVE_TIME) {
    digitalWrite(output1, LOW);
    outputActive[1] = false;
  }
}

void checkState() {
  if(millis() - timer_state > CONTROL_LOOP_TIME) {
    if(targetTemperature - temperature > 2.0 || temperature < temperatureOld) {
      // Open for CONTROL_ACTIVE_TIME
      if(temperature > 0 && outputActive[0] == false && outputActive[1] == false) {
        digitalWrite(output0, HIGH);
        outputActive[0] = true;
        activated = millis();
      }
    } else if (targetTemperature - temperature < -2.0 || temperature > temperatureOld) {
      // Close for CONTROL_ACTIVE_TIME
      if(temperature > 0 && outputActive[0] == false && outputActive[1] == false) {
        digitalWrite(output1, HIGH);
        outputActive[1] = true;
        activated = millis();
      }
    }
    timer_state = millis();
    dT = (temperature - temperatureOld) / CONTROL_LOOP_TIME * 60.0 * 1000.0;
    temperatureOld = temperature;
  }
  if (temperature <= 0) {
    digitalWrite(output0, LOW);
    digitalWrite(output1, LOW);
    outputActive[0] = false;
    outputActive[1] = false;
  }
}

void readButton() {
  // If timer, read button (button deadtime)
  // if pressed, increase temperature by one in range [20,37]
  if(millis() - timer_button > BUTTON_DEAD_TIME) {
    int button = lcd.button();
    if (button == 1) {
      targetTemperature = (targetTemperature - 20 + 1)%18 + 20;
      timer_button = millis();
    } else if (button == 2) {
      targetTemperature--;
      if(targetTemperature == 19) {
        targetTemperature = 37;
      }
      timer_button = millis();
    }
  }
}

void readTemperature() {
  if(millis() - timer_temperature > 1000) {
    sensors.requestTemperatures();
    temperature = sensors.getTempCByIndex(0);
    timer_temperature = millis();
  }
}