#include <Wire.h>
#include "Adafruit_DRV2605.h"

#define PRESSURE_SENSOR_PIN A0

Adafruit_DRV2605 drv;

unsigned long lastTime = 0;
const unsigned long interval = 70; // adjust between 50-80ms to taste

void setup() {
  Serial.begin(9600);
  Serial.println("DRV2605 Pressure Feedback Initializing...");

  if (!drv.begin()) {
    Serial.println("Could not find DRV2605");
    while (1) delay(10);
  }

  drv.selectLibrary(1);
  drv.setMode(DRV2605_MODE_INTTRIG); 
}

void loop() {
  if (millis() - lastTime >= interval) {
    lastTime = millis();

    int analogReading = analogRead(PRESSURE_SENSOR_PIN);

    int constrainedReading = constrain(analogReading, 0, 820);
    uint8_t effect = map(constrainedReading, 0, 820, 0, 117);

    Serial.print("Pressure: ");
    Serial.print(analogReading);
    Serial.print(" -> Triggering Effect #");
    Serial.println(effect);

  if (analogReading > 20) {
      drv.setWaveform(0, effect);
      drv.setWaveform(1, effect);
      drv.setWaveform(2, effect);
      drv.setWaveform(3, effect);
      drv.setWaveform(4, 0);
      drv.go();
    } else {
      drv.stop(); // immediately halt motor when pressure is released
    }
  }
}
