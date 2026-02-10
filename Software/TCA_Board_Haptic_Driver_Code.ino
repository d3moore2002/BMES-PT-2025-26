#include <Wire.h>
#include "Adafruit_DRV2605.h"

Adafruit_DRV2605 drv;  // one object is fine if you select mux channel before using it

void TCA9548A(uint8_t bus) {
  Wire.beginTransmission(0x70);   // default TCA9548A address
  Wire.write(1 << bus);           // select channel
  Wire.endTransmission();
}

void initDrvOnChannel(uint8_t ch) {
  TCA9548A(ch);
  Serial.print("Init DRV2605 on channel ");
  Serial.println(ch);

  if (!drv.begin()) {
    Serial.print("Could not find DRV2605 on channel ");
    Serial.println(ch);
    while (1) delay(10);
  }

  drv.selectLibrary(1);
  drv.setMode(DRV2605_MODE_INTTRIG);
}

uint8_t effect = 1;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(100);

  initDrvOnChannel(0);
  initDrvOnChannel(6);
}

void playEffectOnChannel(uint8_t ch, uint8_t effect) {
  TCA9548A(ch);
  drv.setWaveform(0, effect);
  drv.setWaveform(1, 0);
  drv.go();
}

void loop() {
  // play on both drivers
  playEffectOnChannel(0, effect);
  delay(50);                 // small gap so I2C transactions don't overlap too tightly
  playEffectOnChannel(6, effect);

  delay(500);

  effect++;
  if (effect > 3) effect = 1;
}
