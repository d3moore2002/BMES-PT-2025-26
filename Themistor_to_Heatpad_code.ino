#define ntc_pin A0         // Pin,to which the voltage divider is connected
#define vd_power_pin 2        // 5V for the voltage divider
#define heatpad_pin 3

#define nominal_resistance 10000       //Nominal resistance at 25⁰C
#define nominal_temeprature 25   // temperature for nominal resistance (almost always 25⁰ C)
#define samplingrate 5    // Number of samples
#define beta 3950  // The beta coefficient or the B value of the thermistor (usually 3000-4000) check the datasheet for the accurate value.
#define Rref 10000   //Value of  resistor used for the voltage divider

// #define temp_on 58 // temperature threshhold to turn ON 
// #define temp_off 63 // temperature where heater turns OFF 
#define max_temp 63
#define mid_temp1 55
#define mid_temp2 60 
#define low_temp 45

int samples = 0;   //array to store the samples
// bool heater_state = false;

void setup(void) {
  pinMode(vd_power_pin, OUTPUT);
  Serial.begin(9600);   //initialize serial communication at a baud rate of 9600
  pinMode(heatpad_pin, OUTPUT);
}
void loop(void) {
  uint8_t i;
  float average;
  samples = 0;
  // take voltage readings from the voltage divider
  digitalWrite(vd_power_pin, HIGH);
  delay(5); // small delay to stabilize ADC 

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

  int heater_power;

  if (temperature<low_temp){
    heater_power = 255; //full power 
  }
  else if (temperature<mid_temp1){
    heater_power = 180; //70%
  }
  else if (temperature<mid_temp2){
    heater_power = 100; //40%
  }
  else if (temperature<max_temp){
    heater_power = 40; //low power 
  }
  else {
    heater_power = 0; //no heating
  }

  // if (temperature < temp_on){
  //   heater_state = false; 
  // }

  // if (heater_state){
  //   analogWrite(heatpad_pin, 255); // heater ON 
  // }
  //   else {
  //     analogWrite(heatpad_pin, 0); // heater off
  //   }
  
  // analogWrite(heatpad_pin, 0);
  Serial.print("Temperature ");
  Serial.print(temperature);
  Serial.println(" *C");

  Serial.print("Heater: ");
  if (heater_state) {
    Serial.println("ON");
  }
  else {
    Serial.println("OFF");
  }
  delay(2000);
}