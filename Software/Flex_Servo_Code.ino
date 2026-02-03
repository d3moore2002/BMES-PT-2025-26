#include <Servo.h>

Servo myservo;

const int FLEX_PIN = A0; // Pin connected to voltage divider output

// Measure the voltage at 5V and the actual resistance of your
// 47k resistor, and enter them below:
const float VCC = 4.98;      // Measured voltage of Arduino 5V line
const float R_DIV = 47500.0; // Measured resistance of 47k resistor

// Upload the code, then try to adjust these values to more
// accurately calculate bend degree.
const float STRAIGHT_RESISTANCE = 87500.0; // resistance when straight
const float BEND_RESISTANCE = 33300.0;     // resistance at 90 deg

int pos = 0;

void setup() {
  Serial.begin(9600);
  pinMode(FLEX_PIN, INPUT);
  myservo.attach(11);
  myservo.write(0);
}

void loop() {
  // Read the ADC and calculate voltage and resistance
  int flexADC = analogRead(FLEX_PIN);
  float flexV = flexADC * VCC / 1023.0;
  float flexR = R_DIV * (VCC / flexV - 1.0);

  Serial.println("Resistance: " + String(flexR) + " ohms");

  // Estimate bend angle based on resistance
  float angle = map(flexR, STRAIGHT_RESISTANCE, BEND_RESISTANCE, 0, 90.0);

  Serial.println("Bend: " + String(angle) + " degrees");
  Serial.println();

  delay(500);

  if (angle>0 && angle<45) { // if flex sensor bent angle is within 45 degrees
    // in steps of 1 degree
    myservo.write(45);              // tell servo to go to position 45
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  if (angle>45 && angle<90) { // if flex sensor bent angle is 45 - 90 degrees
    // in steps of 1 degree
    myservo.write(90);              // tell servo to go to position 90
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}