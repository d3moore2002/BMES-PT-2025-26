#include <Servo.h>

Servo myservo;
Servo myservo2;
Servo myservo3;

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


int pos = 0;

void setup() {
  Serial.begin(9600);
  pinMode(FLEX_PIN, INPUT);
  pinMode(FLEX_PIN2, INPUT);
  pinMode(FLEX_PIN3, INPUT);
  myservo.attach(11);
  myservo.write(0);
  myservo2.attach(10);
  myservo2.write(0);
  myservo3.attach(9);
  myservo3.write(0);
}

void loop() {
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
  delay(500);

  myservo.write(angle*2);
  myservo2.write(angle2*2);
  myservo3.write(angle3*2);
  delay(500);
}
