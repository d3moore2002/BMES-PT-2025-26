#include <Wire.h>
#include "Adafruit_DRV2605.h"

// ── Multiplexer ──────────────────────────────────────────────
#define MUX_ADDR       0x70   // TCA9548A default I2C address
#define MUX_CH_DRV0    0      // Channel 0 → first  DRV2605L
#define MUX_CH_DRV1    1      // Channel 1 → second DRV2605L

// ── Pressure sensors ─────────────────────────────────────────
#define PRESSURE_PIN_0 A0     // Sensor for motor 0
#define PRESSURE_PIN_1 A1     // Sensor for motor 1
#define PRESSURE_THRESHOLD 20 // Readings at or below this = no pressure

// ── Timing ───────────────────────────────────────────────────
const unsigned long INTERVAL = 70; // ms between updates (50–80 recommended)
unsigned long lastTime = 0;

// ── Two independent driver instances ─────────────────────────
Adafruit_DRV2605 drv0;
Adafruit_DRV2605 drv1;

// ════════════════════════════════════════════════════════════
//  Helper: route I2C bus to one multiplexer channel
//  Pass -1 to disable all channels
// ════════════════════════════════════════════════════════════
void selectMuxChannel(int8_t ch) {
  Wire.beginTransmission(MUX_ADDR);
  Wire.write((ch >= 0) ? (1 << ch) : 0x00);
  Wire.endTransmission();
}

// ════════════════════════════════════════════════════════════
//  Helper: initialise one DRV2605L on the chosen mux channel
// ════════════════════════════════════════════════════════════
bool initDriver(Adafruit_DRV2605 &drv, uint8_t muxCh, const char *label) {
  selectMuxChannel(muxCh);
  if (!drv.begin()) {
    Serial.print("ERROR – could not find ");
    Serial.println(label);
    return false;
  }
  drv.selectLibrary(1);
  drv.setMode(DRV2605_MODE_INTTRIG);
  Serial.print(label);
  Serial.println(" initialised OK");
  return true;
}

// ════════════════════════════════════════════════════════════
//  Helper: read pressure → play/stop one motor
// ════════════════════════════════════════════════════════════
void updateMotor(Adafruit_DRV2605 &drv,
                 uint8_t          muxCh,
                 uint8_t          sensorPin,
                 const char      *label)
{
  selectMuxChannel(muxCh);               // aim I2C at this driver

  int raw        = analogRead(sensorPin);
  int constrained = constrain(raw, 0, 820);
  uint8_t effect  = map(constrained, 0, 820, 0, 117);

  Serial.print(label);
  Serial.print(" | Pressure: ");
  Serial.print(raw);
  Serial.print(" -> Effect #");
  Serial.println(effect);

  if (raw > PRESSURE_THRESHOLD) {
    drv.setWaveform(0, effect);
    drv.setWaveform(1, effect);
    drv.setWaveform(2, effect);
    drv.setWaveform(3, effect);
    drv.setWaveform(4, 0);        // terminate waveform sequence
    drv.go();
  } else {
    drv.stop();
  }
}

// ════════════════════════════════════════════════════════════
//  SETUP
// ════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(9600);
  Wire.begin();
  Serial.println("Dual DRV2605L + TCA9548A Initialising...");

  // ── Quick I2C scan (useful for debugging wiring) ──────────
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

  // ── Initialise each driver through its mux channel ───────
  initDriver(drv0, MUX_CH_DRV0, "DRV0 (A0)");
  initDriver(drv1, MUX_CH_DRV1, "DRV1 (A1)");

  // Disable all mux channels when idle (good practice)
  selectMuxChannel(-1);
  Serial.println("Ready.\n");
}

// ════════════════════════════════════════════════════════════
//  LOOP
// ════════════════════════════════════════════════════════════
void loop() {
  if (millis() - lastTime >= INTERVAL) {
    lastTime = millis();

    updateMotor(drv0, MUX_CH_DRV0, PRESSURE_PIN_0, "Motor0");
    updateMotor(drv1, MUX_CH_DRV1, PRESSURE_PIN_1, "Motor1");

    selectMuxChannel(-1);   // idle: disable all channels
  }
}

