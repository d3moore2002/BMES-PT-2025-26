#include <Wire.h>
#include "Adafruit_DRV2605.h"

#define PRESSURE_SENSOR_PIN A0

Adafruit_DRV2605 drv;

void setup() {
  Serial.begin(9600);
  Serial.println("DRV2605 Pressure Feedback Initializing...");

  if (!drv.begin()) {
    Serial.println("Could not find DRV2605");
    while (1) delay(10);
  }

  drv.selectLibrary(1);
  // Internal trigger mode allows us to fire effects via the 'go' command
  drv.setMode(DRV2605_MODE_INTTRIG); 
}

void loop() {
  // 1. Read the pressure sensor (Value between 0 and 1023)
  int analogReading = analogRead(PRESSURE_SENSOR_PIN);

  // 2. Map the pressure to an effect ID
  // We map 0-1023 to 0-117. 
  // Effect 0 is "Stop", 1-117 are various intensities.
  uint8_t effect = map(analogReading, 0, 1023, 0, 117);

  // 3. Provide Serial Feedback
  Serial.print("Pressure: ");
  Serial.print(analogReading);
  Serial.print(" -> Triggering Effect #");
  Serial.println(effect);

  // 4. Trigger the vibration if there is enough pressure
  // We add a small "deadzone" (threshold of 20) so it doesn't vibrate constantly
  if (analogReading > 20) {
    drv.setWaveform(0, effect);  // Play the mapped effect
    drv.setWaveform(1, 0);       // End waveform signifier
    drv.go();                    // Play it!
  }

  // 5. Short delay to prevent overwhelming the I2C bus 
  // and to let the haptic motor finish its cycle
  delay(200); 
}
