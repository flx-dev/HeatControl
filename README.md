# Arduino based heat stabilizer

## Overview

Reads temperature from DS18B20 Sensor via DallasTemperature and OneWire library.
Uses up and down button from shield to adjust the target temperature
in range [20,38]. Shield displays current temperature, target temperature,
temperature derivative, output open isActive and output close isActive.

## Getting Startet

Clone repo, open either with VSCode or Arduino IDE. Should work with every
Arduino capable editor. Adjust pin constants at 
```C
#define OUTPUT0 11
#define OUTPUT1 12
#define TEMPPIN 12
```
where `OUTPUT0` is the motor open gate (true means active) and `OUTPUT1` is the close gate.

## Output

Has two digital outputs defined for controlling the heater. As mentioned above they are
labelled `OUTPUT0` and `OUTPUT1`. Never open both outputs at the same time. If DS18B20 is not
connected, the library will retrieve a value of -127 degree. The controller closes all
gates if temperature < 0.

## Dependencies
#### Arduino Libraries
- LiquidCrystal
- OneWire
- DallasTemperature
#### External
- https://github.com/dzindra/LCDKeypad