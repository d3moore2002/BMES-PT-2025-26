#include <Wire.h>
#include "Adafruit_DRV2605.h"

// ── Multiplexer ───────────────────────────────────────────────
#define MUX_ADDR 0x70

// ── Config: tie each pair together in one struct ──────────────
struct HapticPair {
  uint8_t          muxChannel;
  uint8_t          sensorPin;
  const char*      label;
  Adafruit_DRV2605 drv;
};

// Five independent pairs — add or remove rows to scale
HapticPair pairs[] = {
  { 0, A0, "Motor0" },
  { 1, A1, "Motor1" },
  { 2, A2, "Motor2" },
  { 3, A3, "Motor3" },
  { 4, A5, "Motor4" },
};
const uint8_t NUM_PAIRS = sizeof(pairs) / sizeof(pairs[0]);

// ── Timing ────────────────────────────────────────────────────
const unsigned long INTERVAL         = 70;
const int           PRESSURE_THRESH  = 20;
unsigned long       lastTime         = 0;

// ════════════════════════════════════════════════════════════
//  Mux helper
// ════════════════════════════════════════════════════════════
void selectMuxChannel(int8_t ch) {
  Wire.beginTransmission(MUX_ADDR);
  Wire.write((ch >= 0) ? (1 << ch) : 0x00);
  Wire.endTransmission();
  delayMicroseconds(200);
}

// ════════════════════════════════════════════════════════════
//  SETUP
// ════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(9600);
  Wire.begin();
  Serial.println("5-Pair Haptic System Initialising...");

  // I2C bus scan
  Serial.println("Scanning I2C bus...");
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("  Found device at 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
    }
  }
  Serial.println("Scan complete.");

  // Init each driver through its mux channel
  for (uint8_t i = 0; i < NUM_PAIRS; i++) {
    selectMuxChannel(pairs[i].muxChannel);
    if (!pairs[i].drv.begin()) {
      Serial.print("ERROR – could not find ");
      Serial.println(pairs[i].label);
    } else {
      pairs[i].drv.selectLibrary(1);
      pairs[i].drv.setMode(DRV2605_MODE_INTTRIG);
      Serial.print(pairs[i].label);
      Serial.println(" initialised OK");
    }
  }

  selectMuxChannel(-1);
  Serial.println("Ready.\n");
}

// ════════════════════════════════════════════════════════════
//  LOOP
// ════════════════════════════════════════════════════════════
void loop() {
  if (millis() - lastTime >= INTERVAL) {
    lastTime = millis();

    for (uint8_t i = 0; i < NUM_PAIRS; i++) {
      selectMuxChannel(pairs[i].muxChannel);

      int raw         = analogRead(pairs[i].sensorPin);
      int bounded     = constrain(raw, 0, 820);
      uint8_t effect  = map(bounded, 0, 820, 0, 117);

      Serial.print(pairs[i].label);
      Serial.print(" | Pressure: ");
      Serial.print(raw);
      Serial.print(" -> Effect #");
      Serial.println(effect);

      if (raw > PRESSURE_THRESH) {
        pairs[i].drv.setWaveform(0, effect);
        pairs[i].drv.setWaveform(1, effect);
        pairs[i].drv.setWaveform(2, effect);
        pairs[i].drv.setWaveform(3, effect);
        pairs[i].drv.setWaveform(4, 0);
        pairs[i].drv.go();
      } else {
        pairs[i].drv.stop();
      }
    }

    selectMuxChannel(-1);
  }
}
