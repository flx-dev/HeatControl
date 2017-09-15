# Arduino based heat stabilizer

Reads temperature from DS18B20 Sensor via DallasTemperature and OneWire library.
Uses up and down button from shield to adjust the target temperature
in range [20,38]. Shield displays current temperature, target temperature,
temperature derivative, output open isActive and output close isActive.

### Output

Has two digital outputs for controlling the heater.

### Dependencies
#### Arduino Libraries
- LiquidCrystal
- OneWire
- DallasTemperature
#### External
- https://github.com/dzindra/LCDKeypad