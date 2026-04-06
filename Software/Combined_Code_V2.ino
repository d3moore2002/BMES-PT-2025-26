#include <Servo.h>
#include <Wire.h>
#include "Adafruit_DRV2605.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define ntc_pin A0
#define vd_power_pin 2
#define heatpad_pin 3
#define nominal_resistance 10000
#define nominal_temeprature 25
#define samplingrate 5
#define beta 3950
#define Rref 10000

#define MUX_ADDR       0x70
#define MUX_CH_DRV0    0
#define MUX_CH_DRV1    1

#define PRESSURE_PIN_0 A8
#define PRESSURE_PIN_1 A9
#define PRESSURE_THRESHOLD 20

const int FLEX_PIN  = A1;
const int FLEX_PIN2 = A2;
const int FLEX_PIN3 = A3;

const int servoPin = 7;
const int potPin   = A10;

const unsigned long MOTOR_INTERVAL = 70;
const unsigned long THERM_INTERVAL = 500;

const float VCC = 4.98;
const float R_DIV = 47500.0;

const float STRAIGHT_RESISTANCE  = 71599.0;
const float BEND_RESISTANCE      = 84590.0;
const float STRAIGHT_RESISTANCE2 = 65000.0;
const float BEND_RESISTANCE2     = 15000.0;
const float STRAIGHT_RESISTANCE3 = 71000.0;
const float BEND_RESISTANCE3     = 19000.0;

const float setpointC = 35.0;
const float hysteresisC = 1.0;
const int heaterPWM = 200;

int samples = 0;
bool heaterOn = false;

Servo myservo;
Servo myservo2;
Servo myservo3;
Servo servo;
Servo potServo;

Adafruit_DRV2605 drv0;
Adafruit_DRV2605 drv1;
Adafruit_MPU6050 srituhobby;

unsigned long lastMotorTime = 0;
unsigned long lastThermTime = 0;

void selectMuxChannel(int8_t ch) {
  Wire.beginTransmission(MUX_ADDR);
  Wire.write((ch >= 0) ? (1 << ch) : 0x00);
  Wire.endTransmission();
}

bool initDriver(Adafruit_DRV2605 &drv, uint8_t muxCh, const char *label) {
  selectMuxChannel(muxCh);

  if (!drv.begin()) {
    Serial.print("ERROR: ");
    Serial.print(label);
    Serial.println(" not found");
    return false;
  }

  drv.selectLibrary(1);
  drv.setMode(DRV2605_MODE_INTTRIG);

  Serial.print(label);
  Serial.println(" ready");
  return true;
}

int clampServoAngle(int angle) {
  if (angle < 0) return 0;
  if (angle > 180) return 180;
  return angle;
}

float readFlexResistance(int pin) {
  int flexADC = analogRead(pin);
  if (flexADC <= 0) flexADC = 1;

  float flexV = flexADC * VCC / 1023.0;
  if (flexV <= 0.001) flexV = 0.001;

  float flexR = R_DIV * (VCC / flexV - 1.0);
  return flexR;
}

int flexToAngle(float flexR, float straightR, float bendR) {
  float angle = map((long)flexR, (long)straightR, (long)bendR, 0, 90);
  int servoAngle = (int)(angle * 2.0);
  return clampServoAngle(servoAngle);
}

void updateMotor(Adafruit_DRV2605 &drv, uint8_t muxCh, uint8_t sensorPin, const char *label) {
  selectMuxChannel(muxCh);

  int raw = analogRead(sensorPin);
  int constrainedVal = constrain(raw, 0, 820);
  uint8_t effect = map(constrainedVal, 0, 820, 1, 117);

  Serial.print(label);
  Serial.print(" | Pressure: ");
  Serial.print(raw);
  Serial.print(" | Effect: ");
  Serial.println(effect);

  if (raw > PRESSURE_THRESHOLD) {
    drv.setWaveform(0, effect);
    drv.setWaveform(1, 0);
    drv.go();
  } else {
    drv.stop();
  }
}

float readThermistorC() {
  samples = 0;

  digitalWrite(vd_power_pin, HIGH);
  delay(5);

  for (uint8_t i = 0; i < samplingrate; i++) {
    samples += analogRead(ntc_pin);
    delay(10);
  }

  digitalWrite(vd_power_pin, LOW);

  float adc = samples / (float)samplingrate;

  if (adc <= 0.5) adc = 0.5;
  if (adc >= 1022.5) adc = 1022.5;

  float rTherm = Rref / (1023.0 / adc - 1.0);

  float temperature = rTherm / nominal_resistance;
  temperature = log(temperature);
  temperature /= beta;
  temperature += 1.0 / (nominal_temeprature + 273.15);
  temperature = 1.0 / temperature;
  temperature -= 273.15;

  return temperature;
}

void updateThermistor() {
  float temperature = readThermistorC();

  if (temperature <= (setpointC - hysteresisC)) {
    heaterOn = true;
  } else if (temperature >= (setpointC + hysteresisC)) {
    heaterOn = false;
  }

  analogWrite(heatpad_pin, heaterOn ? heaterPWM : 0);

  Serial.print("Temperature ");
  Serial.print(temperature, 2);
  Serial.print(" *C | Heater: ");
  Serial.println(heaterOn ? "ON" : "OFF");
}

void setup() {
  Serial.begin(115200);

  pinMode(FLEX_PIN, INPUT);
  pinMode(FLEX_PIN2, INPUT);
  pinMode(FLEX_PIN3, INPUT);

  myservo.attach(8);
  myservo2.attach(10);
  myservo3.attach(9);

  myservo.write(0);
  myservo2.write(0);
  myservo3.write(0);

  pinMode(vd_power_pin, OUTPUT);
  pinMode(heatpad_pin, OUTPUT);
  digitalWrite(vd_power_pin, LOW);
  analogWrite(heatpad_pin, 0);

  Wire.begin();

  Serial.println("Initializing DRV2605 drivers...");
  initDriver(drv0, MUX_CH_DRV0, "DRV0");
  initDriver(drv1, MUX_CH_DRV1, "DRV1");
  selectMuxChannel(-1);

  servo.attach(11);
  servo.write(0);

  if (srituhobby.begin()) {
    srituhobby.setAccelerometerRange(MPU6050_RANGE_8_G);
    srituhobby.setGyroRange(MPU6050_RANGE_500_DEG);
    srituhobby.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println("MPU6050 ready");
  } else {
    Serial.println("MPU6050 not found");
  }

  potServo.attach(servoPin);

  Serial.println("Setup complete");
}

void loop() {
  float flexR1 = readFlexResistance(FLEX_PIN);
  float flexR2 = readFlexResistance(FLEX_PIN2);
  float flexR3 = readFlexResistance(FLEX_PIN3);

  int angle1 = flexToAngle(flexR1, STRAIGHT_RESISTANCE,  BEND_RESISTANCE);
  int angle2 = flexToAngle(flexR2, STRAIGHT_RESISTANCE2, BEND_RESISTANCE2);
  int angle3 = flexToAngle(flexR3, STRAIGHT_RESISTANCE3, BEND_RESISTANCE3);

  myservo.write(angle1);
  myservo2.write(angle2);
  myservo3.write(angle3);

  sensors_event_t a, g, temp;
  srituhobby.getEvent(&a, &g, &temp);
  int value = (int)a.acceleration.y;
  value = map(value, -10, 10, 180, 0);
  value = clampServoAngle(value);
  servo.write(value);

  if (millis() - lastMotorTime >= MOTOR_INTERVAL) {
    lastMotorTime = millis();
    updateMotor(drv0, MUX_CH_DRV0, PRESSURE_PIN_0, "Motor0");
    updateMotor(drv1, MUX_CH_DRV1, PRESSURE_PIN_1, "Motor1");
    selectMuxChannel(-1);
  }

  if (millis() - lastThermTime >= THERM_INTERVAL) {
    lastThermTime = millis();
    updateThermistor();
  }

  int potValue = analogRead(potPin);
  int potangle = map(potValue, 0, 1023, 0, 180);
  potangle = clampServoAngle(potangle);
  potServo.write(potangle);

  delay(5);
}
