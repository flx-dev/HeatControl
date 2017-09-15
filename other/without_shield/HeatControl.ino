/**********************************************
* Date: 2017-09-13
* File: heatcontrol.ino
* Description: Arduino based heat mixture
*   control. Watches every X (e.g. 30) seconds 
*   if temperature out of range or going in
*   wrong direction and adjust the mixture
***********************************************/

#include<OneWire.h>
#include<DallasTemperature.h>

// PIN and CONSTANT Definitions
#define controlPin 3             // Button
#define output0 11               // Open
#define output1 12               // Close
#define tempPin 2                // DS18B20

#define CONTROL_ACTIVE_TIME 1000 // [ms]
#define CONTROL_LOOP_TIME 30000  // [ms]
#define BUTTON_DEAD_TIME 500     // [ms]

// Global Variable Declarations
unsigned long timer_blink_outside = 0;
unsigned long timer_blink[2] = {0, 0};
unsigned long timer_temperature = 0;
unsigned long timer_button = 0;
unsigned long timer_controller = 0;
unsigned long timer_serial = 0;
unsigned long timer_state = 0;
bool state[2] = {false, false};         // LED state, on/off
bool digits[2] = {false, false};        // false = 10, true = 1
bool outputActive[2] = {false, false};
unsigned long activated = 0;
float temperature = 26;                 // Current temperature
float temperatureOld = 26;              // Temperature before CONTROL_LOOP_TIME
int targetTemperature = 26;             // Target temperature
int counter[2] = {0, 0};                // #Blinks until digits
int led[2] = {led1, led2};

OneWire oneWire(tempPin);
DallasTemperature sensors(&oneWire);

void setup() {
  if(DEBUG) Serial.begin(9600);

  // Define Pinmodes
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(controlPin, INPUT_PULLUP);
  pinMode(output0, OUTPUT);
  pinMode(output1, OUTPUT);
  digitalWrite(output0, LOW);
  digitalWrite(output1, LOW);

  sensors.begin();
}

void loop() {
  unsigned long tic = millis();
  blink(0, (int)temperature);
  blink(1, (int)targetTemperature);
  readButton();
  checkState();
  close();
}

void print() {
  if(DEBUG && millis() - timer_serial > 1000) {
    Serial.print("T_IST: ");
    Serial.print(temperature);
    Serial.print(" T_OLD: ");
    Serial.print((int)temperatureOld);
    Serial.print(" T_DIFF: ");
    Serial.print((temperatureOld-temperature));
    Serial.print(" T_SOLL: ");
    Serial.println((int)targetTemperature);

    Serial.print(" Open: ");
    Serial.print(outputActive[0]);
    Serial.print(" Close: ");
    Serial.println(outputActive[1]);
    timer_serial = millis();
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
      // Open for 500ms
      if(outputActive[0] == false && outputActive[1] == false) {
        digitalWrite(output0, HIGH);
        outputActive[0] = true;
        activated = millis();
      }
    } else if (targetTemperature - temperature < -2.0 || temperature > temperatureOld) {
      // Close for 500ms
      if(outputActive[0] == false && outputActive[1] == false) {
        digitalWrite(output1, HIGH);
        outputActive[1] = true;
        activated = millis();
      }
    }
    print();
    timer_state = millis();
    temperatureOld = temperature;
  }
}

void readButton() {
  // If timer, read button (button deadtime)
  // if pressed, increase temperature by one in range [20,38]
  if(millis() - timer_button > BUTTON_DEAD_TIME) {
    int data = digitalRead(controlPin);
    if(data == LOW){
      targetTemperature = (targetTemperature - 20 + 1)%18 + 20;
    }
    timer_button = millis();
  }
}

void readTemperature() {
  // Gets called in blink function, to avoid reading
  // at blinking time.
  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);
}

void blink(int i, int t) {
  unsigned long diff = millis() - timer_blink[i];
  if(state[i] == false) {
    // Turn LED ON
    if(diff > counter[i] * BLINK_DELAY_TIME &&
    counter[i] < floor(t/10.0) &&
    digits[i] == false
    ||
    diff > counter[i] * BLINK_DELAY_TIME + DIGIT_OFFSET_TIME &&
    counter[i] < t%10 &&
    digits[i] == true) {
      digitalWrite(led[i], HIGH);
      state[i] = true;
    }
  }

  if(state[i] == true) {
    // Turn LED OFF
    if(diff > LED_ON_TIME + counter[i] * BLINK_DELAY_TIME &&
    counter[i] < floor(t/10.0) && 
    digits[i] == false
    ||
    diff > LED_ON_TIME + counter[i] * BLINK_DELAY_TIME + DIGIT_OFFSET_TIME &&
    counter[i] < t%10 &&
    digits[i] == true) {
      digitalWrite(led[i], LOW);
      state[i] = false;
      counter[i]++;
    }
  }

  if(digits[i] == false && counter[i] >= floor(t / 10.0)) {
    // Reset timers, flags and counters for
    // the first "one" digit.
    counter[i] = 0;
    digits[i] = true;
  }

  if(diff > BLINK_TOTAL_TIME - 2000 && i == 1) {
    // Read temperature two seconds before end
    readTemperature();
  }

  if(diff > BLINK_TOTAL_TIME) {
    // Reset everything at the end and
    // start again with "ten" digit
    digits[i] = false;
    counter[i] = 0;
    timer_blink[i] = millis();
  }
}

