// INITIALIZATION
//Libraries
#include <Servo.h>
#include <Wire.h>
#include "Adafruit_DRV2605.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
//Definition of Pins
//Heatpad + Thermistor pin definitions
#define ntc_pin A5       // Pin,to which the voltage divider is connected
#define vd_power_pin 2        // 5V for the voltage divider
#define heatpad_pin 3
#define nominal_resistance 10000       //Nominal resistance at 25⁰C
#define nominal_temeprature 25   // temperature for nominal resistance (almost always 25⁰ C)
#define samplingrate 5    // Number of samples
#define beta 3950  // The beta coefficient or the B value of the thermistor (usually 3000-4000) check the datasheet for the accurate value.
#define Rref 10000   //Value of  resistor used for the voltage divider
//Pressure sensor + Vibrational Motor pin definitions
// ── Multiplexer ──────────────────────────────────────────────
#define MUX_ADDR       0x70   // TCA9548A default I2C address
#define MUX_CH_DRV0    0      // Channel 0 → first  DRV2605L
#define MUX_CH_DRV1    1      // Channel 1 → second DRV2605L




// ── Pressure sensors ─────────────────────────────────────────
#define PRESSURE_PIN_0 A3     // Sensor for motor 0
#define PRESSURE_PIN_1 A4     // Sensor for motor 1
#define PRESSURE_THRESHOLD 20 // Readings at or below this = no pressure




//Flex+Servo variables
Servo myservo;
Servo myservo2;
Servo myservo3;


// ── Timing ───────────────────────────────────────────────────
const unsigned long INTERVAL = 70; // ms between updates (50–80 recommended)
unsigned long lastTime = 0;


//Vibrational Motors + Pressure Sensors variables
Adafruit_DRV2605 drv0;
Adafruit_DRV2605 drv1;






//IMU + Servo variables
Servo servo;
Adafruit_MPU6050 srituhobby;




//Potentiometer + Servo variables
Servo potServo;




//Flex + Servo Constant Floats
const int FLEX_PIN = A0; // Pin connected to voltage divider output
const int FLEX_PIN2 = A1; // Pin connected to voltage divider output
const int FLEX_PIN3 = A2; // Pin connected to voltage divider output
// Measure the voltage at 5V and the actual resistance of your
// 47k resistor, and enter them below:
const float VCC = 4.98;      // Measured voltage of Arduino 5V line
const float R_DIV = 47500.0; // Measured resistance of 47k resistor
// Upload the code, then try to adjust these values to more
// accurately calculate bend degree.
const float STRAIGHT_RESISTANCE = 60000.0; // resistance when straight
const float BEND_RESISTANCE = 14000.0;     // resistance at 90 deg
const float STRAIGHT_RESISTANCE2 = 65000.0; // resistance when straight
const float BEND_RESISTANCE2 = 15000.0;     // resistance at 90 deg
const float STRAIGHT_RESISTANCE3 = 71000.0; // resistance when straight
const float BEND_RESISTANCE3 = 19000.0;     // resistance at 90 deg
//Potentiometer + Servo Constant Floats
const int servoPin = 7;   // D7
const int potPin = A8;    // Analog pin A8




//Flex Servo ints
int pos = 0;
//Heatpad + Thermistor
int samples = 0;   //array to store the samples




//Pressure Sensor + Vibrational Motor Methods
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




  int raw = analogRead(sensorPin);
  int constrained = constrain(raw, 0, 820);
  uint8_t effect = map(constrained, 0, 820, 0, 117);




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




void setup(){


// NOTE: TEST if removing some of the functions before the pressure sensor + vibrational motor // void setup are needed in the combined code, if not we could remove.
//Flex + Servo VOID SETUP
Serial.begin(115200);
  pinMode(FLEX_PIN, INPUT);
  pinMode(FLEX_PIN2, INPUT);
  pinMode(FLEX_PIN3, INPUT);
  myservo.attach(11);
  myservo.write(0);
  myservo2.attach(10);
  myservo2.write(0);
  myservo3.attach(9);
  myservo3.write(0);
//—----------------------------------------------------------------------
//Pressure Sensor + Vibrational Motor VOID SETUP
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
  initDriver(drv0, MUX_CH_DRV0, "DRV0 (A6)");
  initDriver(drv1, MUX_CH_DRV1, "DRV1 (A7)");




  // Disable all mux channels when idle (good practice)
  selectMuxChannel(-1);
  Serial.println("Ready.\n");
//—----------------------------------------------------------------------
//IMU + Servo VOID SETUP
 
  servo.attach(8);
  Wire.begin();
  srituhobby.begin();
  servo.write(0);




  srituhobby.setAccelerometerRange(MPU6050_RANGE_8_G);//2_G,4_G,8_G,16_G
  srituhobby.setGyroRange(MPU6050_RANGE_500_DEG);//250,500,1000,2000
  srituhobby.setFilterBandwidth(MPU6050_BAND_21_HZ);




  delay(100);




//—----------------------------------------------------------------------
//Heat Pad + Thermistor VOID SETUP
 pinMode(vd_power_pin, OUTPUT);
 pinMode(heatpad_pin, OUTPUT);
//—----------------------------------------------------------------------
//Potentiometer + Servo VOID SETUP
potServo.attach(servoPin);
}


void loop(){
//Flex + Servo VOID LOOP
// Read the ADC and calculate voltage and resistance
  int flexADC = analogRead(FLEX_PIN);
  float flexV = flexADC * VCC / 1023.0;
  float flexR = R_DIV * (VCC / flexV - 1.0);




  Serial.println("Resistance: " + String(flexR) + " ohms");




  // Estimate bend angle based on resistance
  float angle = map(flexR, STRAIGHT_RESISTANCE, BEND_RESISTANCE, 0, 90.0);




  Serial.println("Bend for flex 1: " + String(angle) + " degrees");
  Serial.println();




  // Read the ADC and calculate voltage and resistance
  int flexADC2 = analogRead(FLEX_PIN2);
  float flexV2 = flexADC2 * VCC / 1023.0;
  float flexR2 = R_DIV * (VCC / flexV2 - 1.0);




  Serial.println("Resistance: " + String(flexR2) + " ohms");




  // Estimate bend angle based on resistance
  float angle2 = map(flexR2, STRAIGHT_RESISTANCE2, BEND_RESISTANCE2, 0, 90.0);




  Serial.println("Bend for flex 2: " + String(angle2) + " degrees");
  Serial.println();




  // Read the ADC and calculate voltage and resistance
  int flexADC3 = analogRead(FLEX_PIN3);
  float flexV3 = flexADC3 * VCC / 1023.0;
  float flexR3 = R_DIV * (VCC / flexV3 - 1.0);




  Serial.println("Resistance: " + String(flexR3) + " ohms");




  // Estimate bend angle based on resistance
  float angle3 = map(flexR3, STRAIGHT_RESISTANCE3, BEND_RESISTANCE3, 0, 90.0);




  Serial.println("Bend for flex 3: " + String(angle3) + " degrees");
  Serial.println();
  delay(2000);




  myservo.write(angle*2);
  myservo2.write(angle2*2);
  myservo3.write(angle3*2);
  delay(500);
//—-----------------------------------------------------------------------
//Heat Pad + Thermistor VOID LOOP
 uint8_t i;
 float average;
 samples = 0;
 // take voltage readings from the voltage divider
 digitalWrite(vd_power_pin, HIGH);
 for (i = 0; i < samplingrate; i++) {
   samples += analogRead(ntc_pin);
   delay(10);
 }
 digitalWrite(vd_power_pin, LOW);
 average = 0;
 average = samples / samplingrate;
 Serial.println("\n \n");
 Serial.print("ADC readings ");
 Serial.println(average);
 // Calculate NTC resistance
 average = 1023 / average - 1;
 average = Rref / average;
 Serial.print("Thermistor resistance ");
 Serial.println(average);
 float temperature;
 temperature = average / nominal_resistance;     // (R/Ro)
 temperature = log(temperature);                  // ln(R/Ro)
 temperature /= beta;                   // 1/B * ln(R/Ro)
 temperature += 1.0 / (nominal_temeprature + 273.15); // + (1/To)
 temperature = 1.0 / temperature;                 // Invert
 temperature -= 273.15;                         // convert absolute temp to C
 analogWrite(heatpad_pin, 0);
 Serial.print("Temperature ");
 Serial.print(temperature);
 Serial.println(" *C");
 delay(2250);
// —-------------------------------------------------------------------------




// IMU SERVO VOID LOOP




  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  srituhobby.getEvent(&a, &g, &temp);




  int value = a.acceleration.y;




  value = map(value,  -10, 10, 180, 0);
  servo.write(value);  
  Serial.println(value);
  //delay(10);




// —-------------------------------------------------------------------
//Vibrational Motor + Pressure Sensor VOID LOOP
if (millis() - lastTime >= INTERVAL) {
    lastTime = millis();




    updateMotor(drv0, MUX_CH_DRV0, PRESSURE_PIN_0, "Motor0");
    updateMotor(drv1, MUX_CH_DRV1, PRESSURE_PIN_1, "Motor1");




    selectMuxChannel(-1);   // idle: disable all channels
  }
//—----------------------------------------------------------------------
//Potentiometer + Servo
int potValue = analogRead(potPin);     // 0–1023
  int potangle = map(potValue, 0, 1023, 0, 180);
  potangle = potangle*2;
  potServo.write(potangle);
  delay(100);
}
